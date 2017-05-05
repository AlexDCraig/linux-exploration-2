/*
 * elevator c-look
 * sources referenced:
 * http://www.makelinux.net/books/lkd2/ch13lev1sec5
 * https://github.com/fusion2004/cop4610/blob/master/lab4/clook-iosched.c
 * https://github.com/lborg019/clook-iosched
 *
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

int diskhead = -1;

struct sstf_data {
	struct list_head queue;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
	elv_dispatch_sort(q, next);
}

static int sstf_dispatch(struct request_queue *q, int force)
{
	// print whether data is being read or written
	char direction;
	struct sstf_data *nd = q->elevator->elevator_data;
	struct request *rq = NULL;
	printk("CLOOK algorithm has reached dispatch function\n");

	rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);

	if (rq)
	{
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q,rq);
		diskhead = blk_rq_pos(rq);
		if (rq_data_dir(rq) == READ)
			direction = 'R';

		else
			direction = 'W';

		printk("CLOOK Dispatch. Direction: %c. Sector: %lu\n", direction, (unsigned long) blk_rq_pos(rq));
		return 1;
	}

	return 0;

}

static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	char direction;
	struct sstf_data *nd = q->elevator->elevator_data;
	struct list_head *cur = NULL;

	//advance cur each time
	list_for_each(cur, &nd->queue)
	{
		struct request *c = list_entry(cur, struct request, queuelist);
		if(blk_rq_pos(rq) > diskhead) // check if request is bigger than disk head
		{
            //keep servicing bigger requests until it comes across a smaller request
            //insert when current is smaller than the head and bigger than the request
			if(blk_rq_pos(c) < diskhead || blk_rq_pos(rq) < blk_rq_pos(c))
            {
                    break;
            }
		}
		//otherwise the request is bigger than the disk head
		else
		{
		//find where current is smaller than the disk head or current smaller than request
			if(blk_rq_pos(c) < diskhead &&
			   blk_rq_pos(rq) < blk_rq_pos(c))
				break;
		}
	}

	//print out whether data is being read or written by checking direction
	if(rq_data_dir(rq) == READ)
		direction = 'R';
	else
		direction = 'W';
	printk("[SSTF] add %c %lu\n", direction, (unsigned long)blk_rq_pos(rq));

	//add to end of queue
	list_add_tail(&rq->queuelist, cur);

}

static struct request *
sstf_former_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_prev_entry(rq, queuelist);
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct sstf_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q,e);

	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);

	if (!nd)
	{
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}

	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);

	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_sstf = {
	.ops = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_former_req_fn		= sstf_former_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);


MODULE_AUTHOR("Nehemiah Edwards");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("C-LOOK IO scheduler");

