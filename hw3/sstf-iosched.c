/*
 * elevator sstf (CLOOK)
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

// Merges Requests
static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
	elv_dispatch_sort(q, next);
}

//Dispatch the first request in queue
static int sstf_dispatch(struct request_queue *q, int force)
{
	struct sstf_data *nd = q->elevator->elevator_data;
	struct request *rq;
	char direction;

	rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
	if (rq) {
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
				
		diskhead = blk_rq_pos(rq);
		
		//DEBUG: prints out whether the scheduler is reading or writing at the time
		if(rq_data_dir(rq) == READ)
			direction = 'R';
		else	
			direction = 'W';
		printk("[SSTF] dsp %c %lu\n", direction, blk_rq_pos(rq));
		
		return 1;
	}
	return 0;
}

//Adds request to the queue
static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;
	struct list_head *cur = NULL;
	char direction;
	
	
	list_for_each(cur, &nd->queue){
		struct request *c = list_entry(cur, struct request, queuelist);
	//checks the cur position against the diskhead and rq position
		if (blk_rq_pos(rq) > diskhead){
			if(blk_rq_pos(c) < diskhead || blk_rq_pos(rq) < blk_rq_pos(c))
				break;
		} else {
			if(blk_rq_pos(c) < diskhead && blk_rq_pos(rq) < blk_rq_pos(c))
				break;
		}
	}			
	list_add_tail(&rq->queuelist, cur);
	
	//DEBUG: prints out whether the scheduler is reading or writing at the time
	if(rq_data_dir(rq) == READ)
		direction = 'R';
	else
		direction = 'W';
	printk("[SSTF] dsp %c %lu\n", direction, blk_rq_pos(rq));	
}


//Gets the former request
static struct request * sstf_former_request(struct request_queue *q, struct request *rq){
	struct sstf_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	
	return list_prev_entry(rq, queuelist);
}

//Initializes the queue
static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct sstf_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
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

// Exits the queue
static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

//struct that contains elevator variables
static struct elevator_type elevator_sstf = {
	.ops = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_former_req_fn		= sstf_former_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "SSTF",
	.elevator_owner = THIS_MODULE,
};

//upon initialization, this function pulls the elevator and basically converts it to sstf functions.
static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

//upon exit of the scheduler, the elevator closes out the sstf elevator
static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);


MODULE_AUTHOR("David Baugh/Justin Sherburne");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sstf IO scheduler");
