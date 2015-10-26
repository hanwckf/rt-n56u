#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/netfilter.h>
#include <linux/tcp.h>
#include <linux/ip.h>

#include <net/netfilter/nf_fp_smb.h>

static __be32 nf_fp_smb_host __read_mostly = 0;

int nf_fp_smb_hook_in(struct sk_buff *skb)
{
	const struct iphdr *iph = ip_hdr(skb);
	const struct tcphdr *tcph;
	struct tcphdr _hdr;

	if (!nf_fp_smb_host)
		return 0;

	if (unlikely(iph->protocol != IPPROTO_TCP))
		return 0;

	if (likely(iph->daddr != nf_fp_smb_host))
		return 0;

	tcph = skb_header_pointer(skb, (iph->ihl<<2), sizeof(_hdr), &_hdr);
	if (!tcph)
		return 0;

	if (tcph->dest == __constant_htons(445) ||
	    tcph->dest == __constant_htons(139)) {
		skb->nf_fp_cache |= NF_FP_CACHE_SMB;
		return 1;
	}

	return 0;
}

int nf_fp_smb_hook_out(struct sk_buff *skb)
{
	const struct iphdr *iph = ip_hdr(skb);
	const struct tcphdr *tcph;
	struct tcphdr _hdr;

	if (!nf_fp_smb_host)
		return 0;

	if (unlikely(iph->protocol != IPPROTO_TCP))
		return 0;

	if (likely(iph->saddr != nf_fp_smb_host))
		return 0;

	tcph = skb_header_pointer(skb, (iph->ihl<<2), sizeof(_hdr), &_hdr);
	if (!tcph)
		return 0;

	if (tcph->source == __constant_htons(445) ||
	    tcph->source == __constant_htons(139)) {
		skb->nf_fp_cache |= NF_FP_CACHE_SMB;
		return 1;
	}

	return 0;
}

#ifdef CONFIG_PROC_FS
static int nf_fp_smb_seq_show(struct seq_file *s, void *v)
{
	return seq_printf(s, "%pI4\n", &nf_fp_smb_host);
}

static ssize_t nf_fp_smb_seq_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char buf[20];
	u32 in[4], smb_ip = 0;
	int i;

	if (count > (sizeof(buf) - 1))
		count = (sizeof(buf) - 1);
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0';

	if (sscanf(buf, "%d.%d.%d.%d", &in[0], &in[1], &in[2], &in[3]) == 4) {
		for (i = 0; i < 4; i++) {
			if (in[i] > 255)
				return count;
		}
		smb_ip = (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
	} else if (sscanf(buf, "%d", &smb_ip) != 1) {
		return count;
	}

	if (smb_ip == (u32)-1)
		return count;

	nf_fp_smb_host = htonl(smb_ip);

	return count;
}

static void *seq_start(struct seq_file *p, loff_t *pos)
{
	return NULL + (*pos == 0);
}

static void *seq_next(struct seq_file *p, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void seq_stop(struct seq_file *p, void *v)
{
}

static const struct seq_operations nf_fp_smb_seq_ops = {
	.start	= seq_start,
	.next	= seq_next,
	.stop	= seq_stop,
	.show	= nf_fp_smb_seq_show,
};

static int nf_fp_smb_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &nf_fp_smb_seq_ops);
}

static const struct file_operations nf_fp_smb_fops = {
	.open		= nf_fp_smb_seq_open,
	.write		= nf_fp_smb_seq_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

#endif

int __init netfilter_fp_smb_init(void)
{
#ifdef CONFIG_PROC_FS
	if (!proc_create("nf_fp_smb", 0664, proc_net_netfilter, &nf_fp_smb_fops))
		return -ENOMEM;
#endif

	return 0;
}

