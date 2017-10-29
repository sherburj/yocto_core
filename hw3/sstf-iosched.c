/*
 * elevator clook
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

//int diskhead = -1;

struct clook_data {
	struct list_head queue;
};

static void clook_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
	//elv_dispatch_sort(q, next);
}

static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *nd = q->elevator->elevator_data;
	//struct request *rq;
	char direction;

	//rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
	if (!list_empty(&nd->queue)/*rq*/) {
		struct request *rq;
		rq = list_entry(nd->queue.next, struct request, queuelist); //get rid of this
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
				
		//diskhead = blk_rq_pos(rq);
		
		//if(rq_data_dir(rq) == READ)
		//	direction = 'R';
		//else	
		//	direction = 'W';
		//printk("[CLOOK] dsp %c %lu\n", direction, blk_rq_pos(rq));
		
		return 1;
	}
	return 0;
}

static void clook_add_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;
	//struck list_head *cur = NULL;
	//char direction;
	
	list_add_tail(&rq->queuelist, &nd->queue); //get rid of me
	
	/*list_for_each(cur, &nd->queue)
	{
		// Set cur to cur list_entry
		struct request *c = list_entry(cur, struct request, queuelist);
		
		// If cur position is greater than diskhead...
		if (blk_rq_pos(rq) > diskhead)
		{
			// If the cur position < diskhead OR rq position < cur position... break and add to cur position
			if(blk_rq_pos(c) < diskhead || blk_rq_pos(rq) < blk_rq_pos(c))
				break;
			
		} else {
			// cur pos less than disk head
			// If the cur position < diskhead AND rq position < cur position... break and add to cur position
			if(blk_rq_pos(c) < diskhead && blk_rq_pos(rq) < blk_rq_pos(c))
				break;
		}
	}
	
	// Add cur request to request queue
	list_add_tail(&rq->queuelist, cur);
	
	// Check direction for read or write
	if(rq_data_dir(rq) == READ)
		direction = 'R';
	else
		direction = 'W';
	printk("[CLOOK] dsp %c %lu\n", direction, blk_rq_pos(rq));
	*/
}

static struct request *
clook_former_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	
//	return list_prev_entry(rq, queuelist);
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static int clook_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct clook_data *nd;
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

static void clook_exit_queue(struct elevator_queue *e)
{
	struct clook_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_clook = {
	.ops = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_former_req_fn		= clook_former_request,
		.elevator_init_fn			= clook_init_queue,
		.elevator_exit_fn			= clook_exit_queue,
	},
	.elevator_name = "clook",
	.elevator_owner = THIS_MODULE,
};

static int __init clook_init(void)
{
	return elv_register(&elevator_clook);
}

static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);


MODULE_AUTHOR("David Baugh/Justin Sherburne");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CLOOK IO scheduler");
