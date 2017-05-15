/*
 * A sample, extra-simple block driver. Updated for kernel 2.6.31.
 *
 * (C) 2003 Eklektix, Inc.
 * (C) 2010 Pat Patterson <pat at superpat dot com>
 * Redistributable under the terms of the GNU GPL.
 */

// This file is an adaptation of Jonathan Corbet's Simple Block Driver
// from https://lwn.net/Articles/58720/ courtesy of Pat Patterson
// We're adding encryption to read/writes of block devices.

// Included files from Original:

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
#include <linux/crypto.h> // Crypto API the teacher wants us to use.

// Addition 1: We need some static variables and a cipher struct for encryption and decryption.
static char* key = "project3key"; // key for encrypt/decrypt operations
static int keyLength = 11; // how long the key is
struct crypto_cipher* cipher; // for encrypt/decrypt operations, defined in crypto.h, this holds a crypto_tfm struct

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

// Alex's addition
// A method to print what's happening in the kernel through the course of R/W request and Enc/Dec operations
// What arises from this operation depends on whether the request in sbd_transfer was a READ or WRITE
void printOperations(unsigned long nbytes, char* operations[], u8* source, u8* destination)
{
	// Handle the pre-operation data.
	unsigned long length = nbytes; // We need to hold the # of bytes occupied by the blocks twice to print properly. nbytes holds the maximum space of all of the blocks
	printk("%s", operations[0]);
	
	while (length--) // start from the total size and move back from there printing the data in unsigned integer format
		printk("%u\n", (unsigned) *src++);


	// Handle the post-operation data.
	length = nbytes; // reset the length variable to its maximum
	printk("%s", operations[1]);

	while (length--) // start from total size and move back from there printing the data in unsigned integer format
		printk("%u\n", (unsigned) *dst++);
	
}

/*
 * Handle an I/O request.
 * Alex changed this from the original to reflect some basic cryptography.
 */
static void sbd_transfer(struct sbd_device *dev, sector_t sector,
		unsigned long nsect, char *buffer, int write) {
	unsigned long offset = sector * logical_block_size; // base memory level, to be used with nbytes to find where in memory we're at
	unsigned long nbytes = nsect * logical_block_size; // add to the offset to find where you're at in memory

	/* Addition 2: We require some extra variables in order to encrypt OR decrypt the proper information.
	- U8 is fancy Linux kernel speak for an unsigned integer that is used here to coordinate where in memory we'll be accessing.
	- If this request is a WRITE operation, then destination is the place in memory where we need to write information, while source is the buffer that contains the information we need to write.
	- If this request is a READ operation, then destination is the buffer we need to read from and source is the place where we store decrypted information.
	- Operation holds whether the information is Encrypted or Unencrypted for printing purposes
	*/
	u8* destination;
	u8* source;
	char* operation[2];

	// Addition 3: Set the key of the cryptographic protocol.
	// Function signature: 
	// crypto_cipher_setkey(struct crypto_cipher cipher, u8* key, unsigned int keyLength)
	crypto_cipher_setkey(cipher, key, keyLength);

	// Error: moved past where is allowed in memory
	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "sbd: Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}

	// WRITE REQUEST. This means we take unencrypted data and encrypt it.
	if (write) 
	{
		operation[0] = "UNENCRYPTED DATA FROM WRITE REQUEST: "; // For printing later
		operation[1] = "ENCRYPTED DATA FROM WRITE REQUEST: ";
		
		destination = dev->data + offset; // This is where ciphertext will be placed. We need to store it in an address that is available within the device struct.
		source = buffer; // This is where the plaintext we will be generating ciphertext from comes from.

		int k = 0;

		// Starting at the initial location of both destination and source, encrypt each block of the plaintext.
		// Move forward by 1 block each time to ensure that no more and no less than a block is being handled at one time.
		// To do so, some handy function signatures:
		// crypto_cipher_blocksize(struct cryto_cipher) -- Takes a cipher struct and returns how much each block is sized.
		// crypto_cipher_encrypt_one(struct cyrpto_cipher, u8* destination, u8* source) -- Encrypt 1 block. Destination is buffer to be filled with encrypted text, source is the buffer to which to read blocks of data from.
		for (k; k < nbytes; k += crypto_cipher_blocksize(cipher));
		{
			crypto_cipher_encrypt_one(cipher, destination + k, source + k);
		}
	
	}

	// READ REQUEST. This means we take encrypted data and decrypt it.
	// Similar, but flipped, logic as above.
	else
	{
		operation[0] = "ENCRYPTED DATA FROM READ REQUEST: "; // for printing
		operation[1] = "DECRYPTED DATA FROM READ REQUEST: ";

		destination = buffer; // Buffer that will be filled with plaintext.
		source = dev->data + buffer; // Buffer that holds the ciphertext.

		int k = 0;

		// Traverse the cipher again on the basis of 1 block at a time, again using the crypto_cipher_blocksize method.
		// This time, let's traverse it and DECRYPT each block of encrypted text back into cipher text.
		// We'll use crypto_cipher_decrypt_one(cipher handle, buffer that will be filled with plaintext, buffer holding the ciphertext)

		for (k; k < nbytes; k += crypto_cipher_blocksize(cipher))
		{
			crypto_cipher_decrypt_one(cipher, destination + k, source + k);
		}
	}

	printOperations(nbytes, operation, source, destination);

	
}

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

/* ALEXS ADDITION: We need to initialize our cipher structure here for later usage in the transfer method.
 */
static int __init sbd_init(void) {
	
	// ALEXS ADDITION: Using Crypto API, allocate a cipher structure for our globally declared cipher 
	// Method: crypto_alloc_cipher(char* algorithm Name, type of cipher, mask)
	// We're using AES encryption/decryption without a mask 
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

// ALEXS ADDITIONS: Free the memory allocated from the cipher structure.
static void __exit sbd_exit(void)
{
	// USING CRYPTO API, de-allocate the memory dedicated to the crypto cipher structure
	// Method: crypto_free_cipher(cipher to be freed)
	crypto_free_cipher(cipher);
	del_gendisk(Device.gd);
	put_disk(Device.gd);
	unregister_blkdev(major_num, "sbd");
	blk_cleanup_queue(Queue);
	vfree(Device.data);
}

module_init(sbd_init);
module_exit(sbd_exit);
