/*
 * A sample, extra-simple block driver. Updated for kernel 2.6.31.
 *
 * (C) 2003 Eklektix, Inc.
 * (C) 2010 Pat Patterson <pat at superpat dot com>
 * Redistributable under the terms of the GNU GPL.
 */

// Additional Sources:
// http://encryptionhowto.sourceforge.net/previous/Encryption-HOWTO-0.2.1-4.html// https://kernel.readthedocs.io/en/sphinx-samples/crypto-API.html
// http://elixir.free-electrons.com/linux/latest/source/Documentation/crypto/api-intro.txt
// And of course the Crypto API itself.

// Files from Pat:
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

// Files I'm including:
#include <linux/crypto.h> // for cryptographic protocols

MODULE_LICENSE("Dual BSD/GPL");
static char *Version = "1.4";

static int major_num = 0;
module_param(major_num, int, 0);
static int logical_block_size = 512;
module_param(logical_block_size, int, 0);
static int nsectors = 1024; /* How big the drive is */
module_param(nsectors, int, 0); 

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE 512

/*
 * Our request queue.
 */
static struct request_queue *Queue;

/*
 * The internal representation of our device.
 */
static struct sbd_device {
	unsigned long size;
	spinlock_t lock;
	u8 *data;
	struct gendisk *gd;
} Device;

// CRYPTO ADDITIONS: 
// 1) Use Crypto API to include a cipher struct to perform encryptions.
// 2) Pass the key and its length to the module through parameters.
// passing the key and its length were suggested by McGrath and this site:
// http://www.chronox.de/crypto-API/crypto/api-skcipher.html
// 3) Set the module parameters.
struct crypto_cipher* cipher;
static char* cryptoKey = "group1004";
static int keyLength = 9;
// Set key as parameter, tell module its char pointer, set permissions
module_param(cryptoKey, charp, 0644);
module_param(keyLength, int, 0644);

/*
 * Handle an I/O request.
 * ** MADE CHANGES HERE: Make it so SBD has encrypted data written to it
 * and decrypted data read from it
 */
static void sbd_transfer(struct sbd_device *dev, sector_t sector,
		unsigned long nsect, char *buffer, int write) {
	
	// dev = struct corresponding to the block device
	// sector = Sector in memory we start with.
	// nsect = How many sectors we've got to read/write
	// buffer = data to be r/w
	// write = Is the request read or write? 
	// offset = Variable that corresponds to how big each sector is so we can traverse through the block.
	// nbytes = Variable that corresponds to how far we've got to traverse.
	
	int k = 0; // loop counter
	u8* dst; // IF W: dst is device, src is block in mem. IF R: dst is block in mem, src is dst.
	u8* src;
	unsigned long offset = sector * logical_block_size;
	unsigned long nbytes = nsect * logical_block_size;

	unsigned long length;

	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "sbd: Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}

	// Set the cipher's key and keyLength.
	crypto_cipher_setkey(cipher, cryptoKey, keyLength);

	/* Idea:
 	* If Write RQ: Traverse thru blocks, transfer blocks of data to device
 	*	Pass block data to device, then encrypt block data.
 	* If Read RQ: Traverse thru device data, write to blocks of memory
 	*/

	if (write)
	{
		src = buffer; // Our source of data to be written is blocks of memory.
		dst = dev->data + offset; // Write this data to a device.

		for (k = 0; k < nbytes; k += crypto_cipher_blocksize(cipher))
		{
			// crypto_cipher_encrypt_one takes three args:
			// 1) Cipher struct.
			// 2) Destination for data.
			// 3) Source of data.
			crypto_cipher_encrypt_one(cipher, dst + k, src + k);
		}

		// Print out unencrypted data, can access thru use of dst ptr.
		printk("I/O WRITE. UNENCRYPTED DATA:\n");
		for (k = 0; k < 100; k++)
			printk("%u", (unsigned) *dst++);

		printk("\n\n");

		// print out encrypted data
		printk("I/O WRITE. ENCRYPTED DATA:\n");
		for (k = 0; k < 100; k++)
			printk("%u", (unsigned) *src++);
	
		printk("\n\n");
	
	}

	// Read Request.
	// Get data from device, put it into blocks.
	// Convert encrypted data --> unencrypted data
	else		
	{
		dst = dev->data + offset;
		src = buffer;

		for (k = 0; k < nbytes; k += crypto_cipher_blocksize(cipher))
		{
			// Same arguments as with encryption, except this method will be reversing the effects of encryption.
			crypto_cipher_decrypt_one(cipher, src + k, dst + k);
		}

		// Print out unencrypted data, can access thru use of dst ptr.
		printk("I/O READ. UNENCRYPTED DATA:\n");
		for (k = 0; k < 100; k++)
			printk("%u", (unsigned) *dst++);

		printk("\n\n");

		// print out encrypted data
		printk("I/O READ. ENCRYPTED DATA:\n");
		for (k = 0; k < 100; k++)
			printk("%u", (unsigned) *src++);
				
		printk("\n\n");

	}
}

// Iterate thru I/O request queue, transfer data from request to device
static void sbd_request(struct request_queue *q) {
	struct request *req;

	req = blk_fetch_request(q);
	while (req != NULL) {
		// blk_fs_request() was removed in 2.6.36 - many thanks to
		// Christian Paro for the heads up and fix...
		//if (!blk_fs_request(req)) {
		if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) {
			printk (KERN_NOTICE "Skip non-CMD request\n");
			__blk_end_request_all(req, -EIO);
			continue;
		}
		sbd_transfer(&Device, blk_rq_pos(req), blk_rq_cur_sectors(req),
				req->buffer, rq_data_dir(req));
		if ( ! __blk_end_request_cur(req, 0) ) {
			req = blk_fetch_request(q);
		}
	}
}

/*
 * The HDIO_GETGEO ioctl is handled in blkdev_ioctl(), which
 * calls this. We need to implement getgeo, since we can't
 * use tools such as fdisk to partition the drive otherwise.
 */
int sbd_getgeo(struct block_device * block_device, struct hd_geometry * geo) {
	long size;

	/* We have no real geometry, of course, so make something up. */
	size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	return 0;
}

/*
 * The device operations structure.
 */
static struct block_device_operations sbd_ops = {
		.owner  = THIS_MODULE,
		.getgeo = sbd_getgeo
};

static int __init sbd_init(void) {
	cipher = crypto_alloc_cipher("aes", 0, 0);

	/*
	 * Set up our internal device.
	 */
	Device.size = nsectors * logical_block_size;
	spin_lock_init(&Device.lock);
	Device.data = vmalloc(Device.size);
	if (Device.data == NULL)
		return -ENOMEM;
	/*
	 * Get a request queue.
	 */
	Queue = blk_init_queue(sbd_request, &Device.lock);
	if (Queue == NULL)
		goto out;
	blk_queue_logical_block_size(Queue, logical_block_size);
	
	/*
	 * Get registered.
	 */
	major_num = register_blkdev(major_num, "sbd");
	if (major_num < 0) {
		printk(KERN_WARNING "sbd: unable to get major number\n");
		goto out;
	}
	/*
	 * And the gendisk structure.
	 */
	Device.gd = alloc_disk(16);
	if (!Device.gd)
		goto out_unregister;
	Device.gd->major = major_num;
	Device.gd->first_minor = 0;
	Device.gd->fops = &sbd_ops;
	Device.gd->private_data = &Device;
	strcpy(Device.gd->disk_name, "sbd0");
	set_capacity(Device.gd, nsectors);
	Device.gd->queue = Queue;
	add_disk(Device.gd);

	return 0;

out_unregister:
	unregister_blkdev(major_num, "sbd");
out:
	vfree(Device.data);
	return -ENOMEM;
}

static void __exit sbd_exit(void)
{
	crypto_free_cipher(cipher);
	del_gendisk(Device.gd);
	put_disk(Device.gd);
	unregister_blkdev(major_num, "sbd");
	blk_cleanup_queue(Queue);
	vfree(Device.data);
}

module_init(sbd_init);
module_exit(sbd_exit);
