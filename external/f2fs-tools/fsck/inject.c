/**
 * inject.c
 *
 * Copyright (c) 2024 OPPO Mobile Comm Corp., Ltd.
 *             http://www.oppo.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <getopt.h>
#include "f2fs.h"
#include "node.h"
#include "inject.h"

/* localtion of a sit/nat entry */
enum entry_pos {
	ENT_IN_JOURNAL,
	ENT_IN_PACK1,
	ENT_IN_PACK2
};

/* format of printing entry position */
#define PRENTPOS "%s %d"
#define show_entry_pos(pack) (pack) == ENT_IN_JOURNAL ? "journal" : "pack", \
			     (pack) == ENT_IN_JOURNAL ? 0 : (pack)

static void print_raw_nat_entry_info(struct f2fs_nat_entry *ne)
{
	if (!c.dbg_lv)
		return;

	DISP_u8(ne, version);
	DISP_u32(ne, ino);
	DISP_u32(ne, block_addr);
}

static void print_raw_sit_entry_info(struct f2fs_sit_entry *se)
{
	int i;

	if (!c.dbg_lv)
		return;

	DISP_u16(se, vblocks);
	if (c.layout)
		printf("%-30s ", "valid_map:");
	else
		printf("%-30s\t\t[", "valid_map");
	for (i = 0; i < SIT_VBLOCK_MAP_SIZE; i++)
		printf("%02x", se->valid_map[i]);
	if (c.layout)
		printf("\n");
	else
		printf("]\n");
	DISP_u64(se, mtime);
}

static void print_raw_sum_entry_info(struct f2fs_summary *sum)
{
	if (!c.dbg_lv)
		return;

	DISP_u32(sum, nid);
	DISP_u8(sum, version);
	DISP_u16(sum, ofs_in_node);
}

static void print_sum_footer_info(struct summary_footer *footer)
{
	if (!c.dbg_lv)
		return;

	DISP_u8(footer, entry_type);
	DISP_u32(footer, check_sum);
}

static void print_node_footer_info(struct node_footer *footer)
{
	if (!c.dbg_lv)
		return;

	DISP_u32(footer, nid);
	DISP_u32(footer, ino);
	DISP_u32(footer, flag);
	DISP_u64(footer, cp_ver);
	DISP_u32(footer, next_blkaddr);
}

static void print_raw_dentry_info(struct f2fs_dir_entry *dentry)
{
	if (!c.dbg_lv)
		return;

	DISP_u32(dentry, hash_code);
	DISP_u32(dentry, ino);
	DISP_u16(dentry, name_len);
	DISP_u8(dentry, file_type);
}

void inject_usage(void)
{
	MSG(0, "\nUsage: inject.f2fs [options] device\n");
	MSG(0, "[options]:\n");
	MSG(0, "  -d debug level [default:0]\n");
	MSG(0, "  -V print the version number and exit\n");
	MSG(0, "  --mb <member name> which member is injected in a struct\n");
	MSG(0, "  --val <new value> new value to set\n");
	MSG(0, "  --str <new string> new string to set\n");
	MSG(0, "  --idx <slot index> which slot is injected in an array\n");
	MSG(0, "  --nid <nid> which nid is injected\n");
	MSG(0, "  --blk <blkaddr> which blkaddr is injected\n");
	MSG(0, "  --sb <0|1|2> --mb <name> [--idx <index>] --val/str <value/string> inject superblock\n");
	MSG(0, "  --cp <0|1|2> --mb <name> [--idx <index>] --val <value> inject checkpoint\n");
	MSG(0, "  --nat <0|1|2> --mb <name> --nid <nid> --val <value> inject nat entry\n");
	MSG(0, "  --sit <0|1|2> --mb <name> --blk <blk> [--idx <index>] --val <value> inject sit entry\n");
	MSG(0, "  --ssa --mb <name> --blk <blk> [--idx <index>] --val <value> inject summary entry\n");
	MSG(0, "  --node --mb <name> --nid <nid> [--idx <index>] --val <value> inject node\n");
	MSG(0, "  --dent --mb <name> --nid <ino> [--dots <1|2>] --val/str <value/string> inject ino's dentry\n");
	MSG(0, "  --dry-run do not really inject\n");

	exit(1);
}

static void inject_sb_usage(void)
{
	MSG(0, "inject.f2fs --sb <0|1|2> --mb <name> [--idx <index>] --val/str <value/string>\n");
	MSG(0, "[sb]:\n");
	MSG(0, "  0: auto select the first super block\n");
	MSG(0, "  1: select the first super block\n");
	MSG(0, "  2: select the second super block\n");
	MSG(0, "[mb]:\n");
	MSG(0, "  magic: inject magic number\n");
	MSG(0, "  s_stop_reason: inject s_stop_reason array selected by --idx <index>\n");
	MSG(0, "  s_errors: inject s_errors array selected by --idx <index>\n");
	MSG(0, "  feature: inject feature\n");
	MSG(0, "  devs.path: inject path in devs array selected by --idx <index> specified by --str <string>\n");
}

static void inject_cp_usage(void)
{
	MSG(0, "inject.f2fs --cp <0|1|2> --mb <name> [--idx <index>] --val <value> inject checkpoint\n");
	MSG(0, "[cp]:\n");
	MSG(0, "  0: auto select the current cp pack\n");
	MSG(0, "  1: select the first cp pack\n");
	MSG(0, "  2: select the second cp pack\n");
	MSG(0, "[mb]:\n");
	MSG(0, "  checkpoint_ver: inject checkpoint_ver\n");
	MSG(0, "  ckpt_flags: inject ckpt_flags\n");
	MSG(0, "  cur_node_segno: inject cur_node_segno array selected by --idx <index>\n");
	MSG(0, "  cur_node_blkoff: inject cur_node_blkoff array selected by --idx <index>\n");
	MSG(0, "  cur_data_segno: inject cur_data_segno array selected by --idx <index>\n");
	MSG(0, "  cur_data_blkoff: inject cur_data_blkoff array selected by --idx <index>\n");
	MSG(0, "  alloc_type: inject alloc_type array selected by --idx <index>\n");
	MSG(0, "  next_blkaddr: inject next_blkaddr of fsync dnodes selected by --idx <index>\n");
	MSG(0, "  crc: inject crc checksum\n");
	MSG(0, "  elapsed_time: inject elapsed_time\n");
}

static void inject_nat_usage(void)
{
	MSG(0, "inject.f2fs --nat <0|1|2> --mb <name> --nid <nid> --val <value> inject nat entry\n");
	MSG(0, "[nat]:\n");
	MSG(0, "  0: auto select the current nat pack\n");
	MSG(0, "  1: select the first nat pack\n");
	MSG(0, "  2: select the second nat pack\n");
	MSG(0, "[mb]:\n");
	MSG(0, "  version: inject nat entry version\n");
	MSG(0, "  ino: inject nat entry ino\n");
	MSG(0, "  block_addr: inject nat entry block_addr\n");
}

static void inject_sit_usage(void)
{
	MSG(0, "inject.f2fs --sit <0|1|2> --mb <name> --blk <blk> [--idx <index>] --val <value> inject sit entry\n");
	MSG(0, "[sit]:\n");
	MSG(0, "  0: auto select the current sit pack\n");
	MSG(0, "  1: select the first sit pack\n");
	MSG(0, "  2: select the second sit pack\n");
	MSG(0, "[mb]:\n");
	MSG(0, "  vblocks: inject sit entry vblocks\n");
	MSG(0, "  valid_map: inject sit entry valid_map\n");
	MSG(0, "  mtime: inject sit entry mtime\n");
}

static void inject_ssa_usage(void)
{
	MSG(0, "inject.f2fs --ssa --mb <name> --blk <blk> [--idx <index>] --val <value> inject summary entry\n");
	MSG(0, "[mb]:\n");
	MSG(0, "  entry_type: inject summary block footer entry_type\n");
	MSG(0, "  check_sum: inject summary block footer check_sum\n");
	MSG(0, "  nid: inject summary entry nid selected by --idx <index\n");
	MSG(0, "  version: inject summary entry version selected by --idx <index\n");
	MSG(0, "  ofs_in_node: inject summary entry ofs_in_node selected by --idx <index\n");
}

static void inject_node_usage(void)
{
	MSG(0, "inject.f2fs --node --mb <name> --nid <nid> [--idx <index>] --val <value> inject node\n");
	MSG(0, "[mb]:\n");
	MSG(0, "  nid: inject node footer nid\n");
	MSG(0, "  ino: inject node footer ino\n");
	MSG(0, "  flag: inject node footer flag\n");
	MSG(0, "  cp_ver: inject node footer cp_ver\n");
	MSG(0, "  next_blkaddr: inject node footer next_blkaddr\n");
	MSG(0, "  i_mode: inject inode i_mode\n");
	MSG(0, "  i_advise: inject inode i_advise\n");
	MSG(0, "  i_inline: inject inode i_inline\n");
	MSG(0, "  i_links: inject inode i_links\n");
	MSG(0, "  i_size: inject inode i_size\n");
	MSG(0, "  i_blocks: inject inode i_blocks\n");
	MSG(0, "  i_xattr_nid: inject inode i_xattr_nid\n");
	MSG(0, "  i_ext.fofs: inject inode i_ext.fofs\n");
	MSG(0, "  i_ext.blk_addr: inject inode i_ext.blk_addr\n");
	MSG(0, "  i_ext.len: inject inode i_ext.len\n");
	MSG(0, "  i_extra_isize: inject inode i_extra_isize\n");
	MSG(0, "  i_inline_xattr_size: inject inode i_inline_xattr_size\n");
	MSG(0, "  i_inode_checksum: inject inode i_inode_checksum\n");
	MSG(0, "  i_compr_blocks: inject inode i_compr_blocks\n");
	MSG(0, "  i_addr: inject inode i_addr array selected by --idx <index>\n");
	MSG(0, "  i_nid: inject inode i_nid array selected by --idx <index>\n");
	MSG(0, "  addr: inject {in}direct node nid/addr array selected by --idx <index>\n");
}

static void inject_dent_usage(void)
{
	MSG(0, "inject.f2fs --dent --mb <name> --nid <nid> [--dots <1|2>] --val/str <value/string> inject dentry\n");
	MSG(0, "[dots]:\n");
	MSG(0, "  1: inject \".\" in directory which is specified by nid\n");
	MSG(0, "  2: inject \"..\" in directory which is specified by nid\n");
	MSG(0, "[mb]:\n");
	MSG(0, "  d_bitmap: inject dentry block d_bitmap of nid\n");
	MSG(0, "  d_hash: inject dentry hash\n");
	MSG(0, "  d_ino: inject dentry ino\n");
	MSG(0, "  d_ftype: inject dentry ftype\n");
	MSG(0, "  filename: inject dentry filename, its hash and len are updated implicitly\n");
}

int inject_parse_options(int argc, char *argv[], struct inject_option *opt)
{
	int o = 0;
	const char *pack[] = {"auto", "1", "2"};
	const char *option_string = "d:Vh";
	char *endptr;
	struct option long_opt[] = {
		{"dry-run", no_argument, 0, 1},
		{"mb", required_argument, 0, 2},
		{"idx", required_argument, 0, 3},
		{"val", required_argument, 0, 4},
		{"str", required_argument, 0, 5},
		{"sb", required_argument, 0, 6},
		{"cp", required_argument, 0, 7},
		{"nat", required_argument, 0, 8},
		{"nid", required_argument, 0, 9},
		{"sit", required_argument, 0, 10},
		{"blk", required_argument, 0, 11},
		{"ssa", no_argument, 0, 12},
		{"node", no_argument, 0, 13},
		{"dent", no_argument, 0, 14},
		{"dots", required_argument, 0, 15},
		{0, 0, 0, 0}
	};

	while ((o = getopt_long(argc, argv, option_string,
				long_opt, NULL)) != EOF) {
		long long val;

		errno = 0;
		switch (o) {
		case 1:
			c.dry_run = 1;
			MSG(0, "Info: Dry run\n");
			break;
		case 2:
			opt->mb = optarg;
			MSG(0, "Info: inject member %s\n", optarg);
			break;
		case 3:
			val = strtoll(optarg, &endptr, 0);
			if (errno != 0 || val >= UINT_MAX || val < 0 ||
			    *endptr != '\0')
				return -ERANGE;
			opt->idx = (unsigned int)val;
			MSG(0, "Info: inject slot index %u\n", opt->idx);
			break;
		case 4:
			opt->val = strtoull(optarg, &endptr, 0);
			if (errno != 0 || *endptr != '\0')
				return -ERANGE;
			MSG(0, "Info: inject value %lld : 0x%llx\n", opt->val,
			    opt->val);
			break;
		case 5:
			opt->str = strdup(optarg);
			if (!opt->str)
				return -ENOMEM;
			MSG(0, "Info: inject string %s\n", opt->str);
			break;
		case 6:
			if (!is_digits(optarg))
				return EWRONG_OPT;
			opt->sb = atoi(optarg);
			if (opt->sb < 0 || opt->sb > 2)
				return -ERANGE;
			MSG(0, "Info: inject sb %s\n", pack[opt->sb]);
			break;
		case 7:
			if (!is_digits(optarg))
				return EWRONG_OPT;
			opt->cp = atoi(optarg);
			if (opt->cp < 0 || opt->cp > 2)
				return -ERANGE;
			MSG(0, "Info: inject cp pack %s\n", pack[opt->cp]);
			break;
		case 8:
			if (!is_digits(optarg))
				return EWRONG_OPT;
			opt->nat = atoi(optarg);
			if (opt->nat < 0 || opt->nat > 2)
				return -ERANGE;
			MSG(0, "Info: inject nat pack %s\n", pack[opt->nat]);
			break;
		case 9:
			val = strtoll(optarg, &endptr, 0);
			if (errno != 0 || val >= UINT_MAX || val < 0 ||
			    *endptr != '\0')
				return -ERANGE;
			opt->nid = (nid_t)val;
			MSG(0, "Info: inject nid %u : 0x%x\n", opt->nid, opt->nid);
			break;
		case 10:
			if (!is_digits(optarg))
				return EWRONG_OPT;
			opt->sit = atoi(optarg);
			if (opt->sit < 0 || opt->sit > 2)
				return -ERANGE;
			MSG(0, "Info: inject sit pack %s\n", pack[opt->sit]);
			break;
		case 11:
			val = strtoll(optarg, &endptr, 0);
			if (errno != 0 || val >= UINT_MAX || val < 0 ||
			    *endptr != '\0')
				return -ERANGE;
			opt->blk = (block_t)val;
			MSG(0, "Info: inject blkaddr %u : 0x%x\n", opt->blk, opt->blk);
			break;
		case 12:
			opt->ssa = true;
			MSG(0, "Info: inject ssa\n");
			break;
		case 13:
			opt->node = true;
			MSG(0, "Info: inject node\n");
			break;
		case 14:
			opt->dent = true;
			MSG(0, "Info: inject dentry\n");
			break;
		case 15:
			opt->dots = atoi(optarg);
			if (opt->dots != TYPE_DOT &&
			    opt->dots != TYPE_DOTDOT)
				return -ERANGE;
			MSG(0, "Info: inject %s dentry\n",
			    opt->dots == TYPE_DOT ? "dot" : "dotdot");
			break;
		case 'd':
			if (optarg[0] == '-' || !is_digits(optarg))
				return EWRONG_OPT;
			c.dbg_lv = atoi(optarg);
			MSG(0, "Info: Debug level = %d\n", c.dbg_lv);
			break;
		case 'V':
			show_version("inject.f2fs");
			exit(0);
		case 'h':
		default:
			if (opt->sb >= 0) {
				inject_sb_usage();
				exit(0);
			} else if (opt->cp >= 0) {
				inject_cp_usage();
				exit(0);
			} else if (opt->nat >= 0) {
				inject_nat_usage();
				exit(0);
			} else if (opt->sit >= 0) {
				inject_sit_usage();
				exit(0);
			} else if (opt->ssa) {
				inject_ssa_usage();
				exit(0);
			} else if (opt->node) {
				inject_node_usage();
				exit(0);
			} else if (opt->dent) {
				inject_dent_usage();
				exit(0);
			} else {
				MSG(0, "\tError: Wrong option -%c (%d) %s\n",
				    o, o, optarg);
			}
			return EUNKNOWN_OPT;
		}
	}

	return 0;
}

static int inject_sb(struct f2fs_sb_info *sbi, struct inject_option *opt)
{
	struct f2fs_super_block *sb;
	char *buf;
	int ret;

	buf = calloc(1, F2FS_BLKSIZE);
	ASSERT(buf != NULL);

	if (opt->sb == 0)
		opt->sb = 1;

	ret = dev_read_block(buf, opt->sb == 1 ? SB0_ADDR : SB1_ADDR);
	ASSERT(ret >= 0);

	sb = (struct f2fs_super_block *)(buf + F2FS_SUPER_OFFSET);

	if (!strcmp(opt->mb, "magic")) {
		MSG(0, "Info: inject magic of sb %d: 0x%x -> 0x%x\n",
		    opt->sb, get_sb(magic), (u32)opt->val);
		set_sb(magic, (u32)opt->val);
	} else if (!strcmp(opt->mb, "s_stop_reason")) {
		if (opt->idx >= MAX_STOP_REASON) {
			ERR_MSG("invalid index %u of sb->s_stop_reason[]\n",
				opt->idx);
			ret = -EINVAL;
			goto out;
		}
		MSG(0, "Info: inject s_stop_reason[%d] of sb %d: %d -> %d\n",
		    opt->idx, opt->sb, sb->s_stop_reason[opt->idx],
		    (u8)opt->val);
		sb->s_stop_reason[opt->idx] = (u8)opt->val;
	} else if (!strcmp(opt->mb, "s_errors")) {
		if (opt->idx >= MAX_F2FS_ERRORS) {
			ERR_MSG("invalid index %u of sb->s_errors[]\n",
				opt->idx);
			ret = -EINVAL;
			goto out;
		}
		MSG(0, "Info: inject s_errors[%d] of sb %d: %x -> %x\n",
		    opt->idx, opt->sb, sb->s_errors[opt->idx], (u8)opt->val);
		sb->s_errors[opt->idx] = (u8)opt->val;
	} else if (!strcmp(opt->mb, "feature")) {
		MSG(0, "Info: inject feature of sb %d: 0x%x -> 0x%x\n",
		    opt->sb, get_sb(feature), (u32)opt->val);
		set_sb(feature, (u32)opt->val);
	} else if (!strcmp(opt->mb, "devs.path")) {
		if (opt->idx >= MAX_DEVICES) {
			ERR_MSG("invalid index %u of sb->devs[]\n", opt->idx);
			ret = -EINVAL;
			goto out;
		}
		if (strlen(opt->str) >= MAX_PATH_LEN) {
			ERR_MSG("invalid length of option str\n");
			ret = -EINVAL;
			goto out;
		}
		MSG(0, "Info: inject devs[%d].path of sb %d: %s -> %s\n",
		    opt->idx, opt->sb, (char *)sb->devs[opt->idx].path, opt->str);
		strcpy((char *)sb->devs[opt->idx].path, opt->str);
	} else {
		ERR_MSG("unknown or unsupported member \"%s\"\n", opt->mb);
		ret = -EINVAL;
		goto out;
	}

	print_raw_sb_info(sb);
	update_superblock(sb, SB_MASK((u32)opt->sb - 1));

out:
	free(buf);
	free(opt->str);
	return ret;
}

static int inject_cp(struct f2fs_sb_info *sbi, struct inject_option *opt)
{
	struct f2fs_checkpoint *cp, *cur_cp = F2FS_CKPT(sbi);
	bool update_crc = true;
	char *buf = NULL;
	int ret = 0;

	if (opt->cp == 0)
		opt->cp = sbi->cur_cp;

	if (opt->cp != sbi->cur_cp) {
		struct f2fs_super_block *sb = sbi->raw_super;
		block_t cp_addr;

		buf = calloc(1, F2FS_BLKSIZE);
		ASSERT(buf != NULL);

		cp_addr = get_sb(cp_blkaddr);
		if (opt->cp == 2)
			cp_addr += 1 << get_sb(log_blocks_per_seg);
		ret = dev_read_block(buf, cp_addr);
		ASSERT(ret >= 0);

		cp = (struct f2fs_checkpoint *)buf;
		sbi->ckpt = cp;
		sbi->cur_cp = opt->cp;
	} else {
		cp = cur_cp;
	}

	if (!strcmp(opt->mb, "checkpoint_ver")) {
		MSG(0, "Info: inject checkpoint_ver of cp %d: 0x%llx -> 0x%"PRIx64"\n",
		    opt->cp, get_cp(checkpoint_ver), (u64)opt->val);
		set_cp(checkpoint_ver, (u64)opt->val);
	} else if (!strcmp(opt->mb, "ckpt_flags")) {
		MSG(0, "Info: inject ckpt_flags of cp %d: 0x%x -> 0x%x\n",
		    opt->cp, get_cp(ckpt_flags), (u32)opt->val);
		set_cp(ckpt_flags, (u32)opt->val);
	} else if (!strcmp(opt->mb, "cur_node_segno")) {
		if (opt->idx >= MAX_ACTIVE_NODE_LOGS) {
			ERR_MSG("invalid index %u of cp->cur_node_segno[]\n",
				opt->idx);
			ret = -EINVAL;
			goto out;
		}
		MSG(0, "Info: inject cur_node_segno[%d] of cp %d: 0x%x -> 0x%x\n",
		    opt->idx, opt->cp, get_cp(cur_node_segno[opt->idx]),
		    (u32)opt->val);
		set_cp(cur_node_segno[opt->idx], (u32)opt->val);
	} else if (!strcmp(opt->mb, "cur_node_blkoff")) {
		if (opt->idx >= MAX_ACTIVE_NODE_LOGS) {
			ERR_MSG("invalid index %u of cp->cur_node_blkoff[]\n",
				opt->idx);
			ret = -EINVAL;
			goto out;
		}
		MSG(0, "Info: inject cur_node_blkoff[%d] of cp %d: 0x%x -> 0x%x\n",
		    opt->idx, opt->cp, get_cp(cur_node_blkoff[opt->idx]),
		    (u16)opt->val);
		set_cp(cur_node_blkoff[opt->idx], (u16)opt->val);
	} else if (!strcmp(opt->mb, "cur_data_segno")) {
		if (opt->idx >= MAX_ACTIVE_DATA_LOGS) {
			ERR_MSG("invalid index %u of cp->cur_data_segno[]\n",
				opt->idx);
			ret = -EINVAL;
			goto out;
		}
		MSG(0, "Info: inject cur_data_segno[%d] of cp %d: 0x%x -> 0x%x\n",
		    opt->idx, opt->cp, get_cp(cur_data_segno[opt->idx]),
		    (u32)opt->val);
		set_cp(cur_data_segno[opt->idx], (u32)opt->val);
	} else if (!strcmp(opt->mb, "cur_data_blkoff")) {
		if (opt->idx >= MAX_ACTIVE_DATA_LOGS) {
			ERR_MSG("invalid index %u of cp->cur_data_blkoff[]\n",
				opt->idx);
			ret = -EINVAL;
			goto out;
		}
		MSG(0, "Info: inject cur_data_blkoff[%d] of cp %d: 0x%x -> 0x%x\n",
		    opt->idx, opt->cp, get_cp(cur_data_blkoff[opt->idx]),
		    (u16)opt->val);
		set_cp(cur_data_blkoff[opt->idx], (u16)opt->val);
	} else if (!strcmp(opt->mb, "alloc_type")) {
		if (opt->idx >= MAX_ACTIVE_LOGS) {
			ERR_MSG("invalid index %u of cp->alloc_type[]\n",
				opt->idx);
			ret = -EINVAL;
			goto out;
		}
		MSG(0, "Info: inject alloc_type[%d] of cp %d: 0x%x -> 0x%x\n",
		    opt->idx, opt->cp, cp->alloc_type[opt->idx],
		    (unsigned char)opt->val);
		cp->alloc_type[opt->idx] = (unsigned char)opt->val;
	} else if (!strcmp(opt->mb, "next_blkaddr")) {
		struct fsync_inode_entry *entry;
		struct list_head inode_list = LIST_HEAD_INIT(inode_list);
		struct f2fs_node *node;
		block_t blkaddr;
		int i = 0;

		if (c.zoned_model == F2FS_ZONED_HM) {
			ERR_MSG("inject fsync dnodes not supported in "
				"zoned device\n");
			ret = -EOPNOTSUPP;
			goto out;
		}

		if (!need_fsync_data_record(sbi)) {
			ERR_MSG("no need to recover fsync dnodes\n");
			ret = -EINVAL;
			goto out;
		}

		ret = f2fs_find_fsync_inode(sbi, &inode_list);
		if (ret) {
			ERR_MSG("failed to find fsync inodes: %d\n", ret);
			goto out;
		}

		list_for_each_entry(entry, &inode_list, list) {
			if (i == opt->idx)
				blkaddr = entry->blkaddr;
			DBG(0, "[%4d] blkaddr:0x%x\n", i++, entry->blkaddr);
		}

		f2fs_destroy_fsync_dnodes(&inode_list);

		if (opt->idx == 0 || opt->idx >= i) {
			ERR_MSG("invalid index %u of fsync dnodes range [1, %u]\n",
				opt->idx, i);
			ret = -EINVAL;
			goto out;
		}

		MSG(0, "Info: inject next_blkaddr[%d] of cp %d: 0x%x -> 0x%x\n",
		    opt->idx, opt->cp, blkaddr, (u32)opt->val);

		node = malloc(F2FS_BLKSIZE);
		ASSERT(node);
		ret = dev_read_block(node, blkaddr);
		ASSERT(ret >= 0);
		F2FS_NODE_FOOTER(node)->next_blkaddr = cpu_to_le32((u32)opt->val);
		if (IS_INODE(node))
			ret = update_inode(sbi, node, &blkaddr);
		else
			ret = update_block(sbi, node, &blkaddr, NULL);
		free(node);
		ASSERT(ret >= 0);
		goto out;
	} else if (!strcmp(opt->mb, "crc")) {
		__le32 *crc = (__le32 *)((unsigned char *)cp +
						get_cp(checksum_offset));

		MSG(0, "Info: inject crc of cp %d: 0x%x -> 0x%x\n",
		    opt->cp, le32_to_cpu(*crc), (u32)opt->val);
		*crc = cpu_to_le32((u32)opt->val);
		update_crc = false;
	} else if (!strcmp(opt->mb, "elapsed_time")) {
		MSG(0, "Info: inject elapsed_time of cp %d: %llu -> %"PRIu64"\n",
		    opt->cp, get_cp(elapsed_time), (u64)opt->val);
		set_cp(elapsed_time, (u64)opt->val);
	} else {
		ERR_MSG("unknown or unsupported member \"%s\"\n", opt->mb);
		ret = -EINVAL;
		goto out;
	}

	print_ckpt_info(sbi);
	write_raw_cp_blocks(sbi, cp, opt->cp, update_crc);

out:
	free(buf);
	sbi->ckpt = cur_cp;
	return ret;
}

static void rewrite_nat_in_journal(struct f2fs_sb_info *sbi, u32 nid,
				   struct f2fs_nat_entry *nat)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = F2FS_SUMMARY_BLOCK_JOURNAL(curseg->sum_blk);
	block_t blkaddr;
	int ret, i;

	for (i = 0; i < nats_in_cursum(journal); i++) {
		if (nid_in_journal(journal, i) == nid) {
			memcpy(&nat_in_journal(journal, i), nat, sizeof(*nat));
			break;
		}
	}

	if (is_set_ckpt_flags(cp, CP_UMOUNT_FLAG))
		blkaddr = sum_blk_addr(sbi, NR_CURSEG_TYPE, CURSEG_HOT_DATA);
	else
		blkaddr = sum_blk_addr(sbi, NR_CURSEG_DATA_TYPE, CURSEG_HOT_DATA);

	ret = dev_write_block(curseg->sum_blk, blkaddr, WRITE_LIFE_NONE);
	ASSERT(ret >= 0);
}

static block_t get_nat_addr(struct f2fs_sb_info *sbi, int nat_pack,
			    nid_t nid, enum entry_pos *pack)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	block_t blkaddr;
	unsigned int blkoff;

	blkoff = NAT_BLOCK_OFFSET(nid);
	blkaddr = nm_i->nat_blkaddr + (blkoff << 1) +
			(blkoff & (sbi->blocks_per_seg - 1));
	if (nat_pack == 0) // select valid nat pack
		nat_pack = f2fs_test_bit(blkoff, nm_i->nat_bitmap) ?
				ENT_IN_PACK2 : ENT_IN_PACK1;
	if (nat_pack == ENT_IN_PACK2)
		blkaddr += sbi->blocks_per_seg;
	if (pack)
		*pack = nat_pack;
	return blkaddr;
}

static struct f2fs_nat_entry *get_raw_nat(struct f2fs_sb_info *sbi,
					  struct inject_option *opt,
					  struct f2fs_nat_block *nat_blk,
					  enum entry_pos *pack)
{
	block_t blkaddr;
	unsigned int offs;

	if (lookup_nat_in_journal(sbi, opt->nid, &nat_blk->entries[0]) >= 0) {
		offs = 0;
		*pack = ENT_IN_JOURNAL;
	} else {
		blkaddr = get_nat_addr(sbi, opt->nat, opt->nid, pack);
		ASSERT(dev_read_block(nat_blk, blkaddr) >= 0);
		offs = opt->nid % NAT_ENTRY_PER_BLOCK;
	}

	return &nat_blk->entries[offs];
}

static void rewrite_raw_nat(struct f2fs_sb_info *sbi,
			    struct inject_option *opt,
			    struct f2fs_nat_block *nat_blk,
			    enum entry_pos pack)
{
	block_t blkaddr;

	if (pack == ENT_IN_JOURNAL) {
		rewrite_nat_in_journal(sbi, opt->nid, &nat_blk->entries[0]);
	} else {
		blkaddr = get_nat_addr(sbi, opt->nat, opt->nid, NULL);
		ASSERT(dev_write_block(nat_blk, blkaddr, WRITE_LIFE_NONE) >= 0);
	}
}

static int inject_nat(struct f2fs_sb_info *sbi, struct inject_option *opt)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_nat_block *nat_blk;
	struct f2fs_nat_entry *ne;
	enum entry_pos pack;

	if (!IS_VALID_NID(sbi, opt->nid)) {
		ERR_MSG("Invalid nid %u range [%u:%"PRIu64"]\n", opt->nid, 0,
			NAT_ENTRY_PER_BLOCK *
			((get_sb(segment_count_nat) << 1) <<
			 sbi->log_blocks_per_seg));
		return -EINVAL;
	}

	nat_blk = calloc(F2FS_BLKSIZE, 1);
	ASSERT(nat_blk);

	ne = get_raw_nat(sbi, opt, nat_blk, &pack);

	if (!strcmp(opt->mb, "version")) {
		MSG(0, "Info: inject nat entry version of nid %u "
		    "in "PRENTPOS": %d -> %d\n", opt->nid,
		    show_entry_pos(pack),
		    ne->version, (u8)opt->val);
		ne->version = (u8)opt->val;
	} else if (!strcmp(opt->mb, "ino")) {
		MSG(0, "Info: inject nat entry ino of nid %u "
		    "in "PRENTPOS": %d -> %d\n", opt->nid,
		    show_entry_pos(pack),
		    le32_to_cpu(ne->ino), (nid_t)opt->val);
		ne->ino = cpu_to_le32((nid_t)opt->val);
	} else if (!strcmp(opt->mb, "block_addr")) {
		MSG(0, "Info: inject nat entry block_addr of nid %u "
		    "in "PRENTPOS": 0x%x -> 0x%x\n", opt->nid,
		    show_entry_pos(pack),
		    le32_to_cpu(ne->block_addr), (block_t)opt->val);
		ne->block_addr = cpu_to_le32((block_t)opt->val);
	} else {
		ERR_MSG("unknown or unsupported member \"%s\"\n", opt->mb);
		free(nat_blk);
		return -EINVAL;
	}
	print_raw_nat_entry_info(ne);

	rewrite_raw_nat(sbi, opt, nat_blk, pack);

	free(nat_blk);
	return 0;
}

static void rewrite_sit_in_journal(struct f2fs_sb_info *sbi, unsigned int segno,
				   struct f2fs_sit_entry *sit)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct f2fs_journal *journal = F2FS_SUMMARY_BLOCK_JOURNAL(curseg->sum_blk);
	block_t blkaddr;
	int ret, i;

	for (i = 0; i < sits_in_cursum(journal); i++) {
		if (segno_in_journal(journal, i) == segno) {
			memcpy(&sit_in_journal(journal, i), sit, sizeof(*sit));
			break;
		}
	}

	if (is_set_ckpt_flags(cp, CP_UMOUNT_FLAG))
		blkaddr = sum_blk_addr(sbi, NR_CURSEG_TYPE, CURSEG_COLD_DATA);
	else
		blkaddr = sum_blk_addr(sbi, NR_CURSEG_DATA_TYPE, CURSEG_COLD_DATA);

	ret = dev_write_block(curseg->sum_blk, blkaddr, WRITE_LIFE_NONE);
	ASSERT(ret >= 0);
}

static block_t get_sit_addr(struct f2fs_sb_info *sbi, int sit_pack,
			    unsigned int segno, enum entry_pos *pack)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int blkaddr, offs;

	offs = SIT_BLOCK_OFFSET(sit_i, segno);
	blkaddr = sit_i->sit_base_addr + offs;
	if (sit_pack == 0) // select valid sit pack
		sit_pack = f2fs_test_bit(offs, sit_i->sit_bitmap) ?
				ENT_IN_PACK2 : ENT_IN_PACK1;
	if (sit_pack == ENT_IN_PACK2)
		blkaddr += sit_i->sit_blocks;
	if (pack)
		*pack = sit_pack;
	return blkaddr;
}

static struct f2fs_sit_entry *get_raw_sit(struct f2fs_sb_info *sbi,
					  struct inject_option *opt,
					  struct f2fs_sit_block *sit_blk,
					  enum entry_pos *pack)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int segno, offs;
	block_t blkaddr;

	segno = GET_SEGNO(sbi, opt->blk);
	if (lookup_sit_in_journal(sbi, segno, &sit_blk->entries[0]) >= 0) {
		offs = 0;
		*pack = ENT_IN_JOURNAL;
	} else {
		blkaddr = get_sit_addr(sbi, opt->sit, segno, pack);
		ASSERT(dev_read_block(sit_blk, blkaddr) >= 0);
		offs = SIT_ENTRY_OFFSET(sit_i, segno);
	}

	return &sit_blk->entries[offs];
}

static void rewrite_raw_sit(struct f2fs_sb_info *sbi,
			    struct inject_option *opt,
			    struct f2fs_sit_block *sit_blk,
			    enum entry_pos pack)
{
	unsigned int segno;
	block_t blkaddr;

	segno = GET_SEGNO(sbi, opt->blk);
	if (pack == ENT_IN_JOURNAL) {
		rewrite_sit_in_journal(sbi, segno, &sit_blk->entries[0]);
	} else {
		blkaddr = get_sit_addr(sbi, opt->sit, segno, NULL);
		ASSERT(dev_write_block(sit_blk, blkaddr, WRITE_LIFE_NONE) >= 0);
	}
}

static int inject_sit(struct f2fs_sb_info *sbi, struct inject_option *opt)
{
	struct f2fs_sit_block *sit_blk;
	struct f2fs_sit_entry *sit;
	enum entry_pos pack;

	if (!f2fs_is_valid_blkaddr(sbi, opt->blk, DATA_GENERIC)) {
		ERR_MSG("Invalid blkaddr 0x%x (valid range [0x%x:0x%lx])\n",
			opt->blk, SM_I(sbi)->main_blkaddr,
			(unsigned long)le64_to_cpu(F2FS_RAW_SUPER(sbi)->block_count));
		return -EINVAL;
	}

	sit_blk = calloc(F2FS_BLKSIZE, 1);
	ASSERT(sit_blk);

	sit = get_raw_sit(sbi, opt, sit_blk, &pack);

	if (!strcmp(opt->mb, "vblocks")) {
		MSG(0, "Info: inject sit entry vblocks of block 0x%x "
		    "in "PRENTPOS": %u -> %u\n", opt->blk,
		    show_entry_pos(pack),
		    le16_to_cpu(sit->vblocks), (u16)opt->val);
		sit->vblocks = cpu_to_le16((u16)opt->val);
	} else if (!strcmp(opt->mb, "valid_map")) {
		if (opt->idx == -1) {
			opt->idx = OFFSET_IN_SEG(sbi, opt->blk);
			MSG(0, "Info: auto idx = %u\n", opt->idx);
		}
		if (opt->idx >= SIT_VBLOCK_MAP_SIZE) {
			ERR_MSG("invalid idx %u of valid_map[]\n", opt->idx);
			free(sit_blk);
			return -ERANGE;
		}
		MSG(0, "Info: inject sit entry valid_map[%d] of block 0x%x "
		    "in "PRENTPOS": 0x%02x -> 0x%02x\n", opt->idx, opt->blk,
		    show_entry_pos(pack),
		    sit->valid_map[opt->idx], (u8)opt->val);
		sit->valid_map[opt->idx] = (u8)opt->val;
	} else if (!strcmp(opt->mb, "mtime")) {
		MSG(0, "Info: inject sit entry mtime of block 0x%x "
		    "in pack %d: %"PRIu64" -> %"PRIu64"\n", opt->blk, opt->sit,
		    le64_to_cpu(sit->mtime), (u64)opt->val);
		sit->mtime = cpu_to_le64((u64)opt->val);
	} else {
		ERR_MSG("unknown or unsupported member \"%s\"\n", opt->mb);
		free(sit_blk);
		return -EINVAL;
	}
	print_raw_sit_entry_info(sit);

	rewrite_raw_sit(sbi, opt, sit_blk, pack);

	free(sit_blk);
	return 0;
}

static int inject_ssa(struct f2fs_sb_info *sbi, struct inject_option *opt)
{
	struct f2fs_summary_block *sum_blk;
	struct summary_footer *footer;
	struct f2fs_summary *sum;
	u32 segno, offset;
	block_t ssa_blkaddr;
	int type;
	int ret;

	if (!f2fs_is_valid_blkaddr(sbi, opt->blk, DATA_GENERIC)) {
		ERR_MSG("Invalid blkaddr %#x (valid range [%#x:%#lx])\n",
			opt->blk, SM_I(sbi)->main_blkaddr,
			(unsigned long)le64_to_cpu(F2FS_RAW_SUPER(sbi)->block_count));
		return -ERANGE;
	}

	segno = GET_SEGNO(sbi, opt->blk);
	offset = OFFSET_IN_SEG(sbi, opt->blk);

	sum_blk = get_sum_block(sbi, segno, &type);
	sum = &sum_blk->entries[offset];
	footer = F2FS_SUMMARY_BLOCK_FOOTER(sum_blk);

	if (!strcmp(opt->mb, "entry_type")) {
		MSG(0, "Info: inject summary block footer entry_type of "
		    "block 0x%x: %d -> %d\n", opt->blk, footer->entry_type,
		    (unsigned char)opt->val);
		footer->entry_type = (unsigned char)opt->val;
	} else 	if (!strcmp(opt->mb, "check_sum")) {
		MSG(0, "Info: inject summary block footer check_sum of "
		    "block 0x%x: 0x%x -> 0x%x\n", opt->blk,
		    le32_to_cpu(footer->check_sum), (u32)opt->val);
		footer->check_sum = cpu_to_le32((u32)opt->val);
	} else {
		if (opt->idx == -1) {
			MSG(0, "Info: auto idx = %u\n", offset);
			opt->idx = offset;
		}
		if (opt->idx >= ENTRIES_IN_SUM) {
			ERR_MSG("invalid idx %u of entries[]\n", opt->idx);
			ret = -EINVAL;
			goto out;
		}
		sum = &sum_blk->entries[opt->idx];
		if (!strcmp(opt->mb, "nid")) {
			MSG(0, "Info: inject summary entry nid of "
			    "block 0x%x: 0x%x -> 0x%x\n", opt->blk,
			    le32_to_cpu(sum->nid), (u32)opt->val);
			sum->nid = cpu_to_le32((u32)opt->val);
		} else if (!strcmp(opt->mb, "version")) {
			MSG(0, "Info: inject summary entry version of "
			    "block 0x%x: %d -> %d\n", opt->blk,
			    sum->version, (u8)opt->val);
			sum->version = (u8)opt->val;
		} else if (!strcmp(opt->mb, "ofs_in_node")) {
			MSG(0, "Info: inject summary entry ofs_in_node of "
			    "block 0x%x: %d -> %d\n", opt->blk,
			    sum->ofs_in_node, (u16)opt->val);
			sum->ofs_in_node = cpu_to_le16((u16)opt->val);
		} else {
			ERR_MSG("unknown or unsupported member \"%s\"\n", opt->mb);
			ret = -EINVAL;
			goto out;
		}

		print_raw_sum_entry_info(sum);
	}

	print_sum_footer_info(footer);

	ssa_blkaddr = GET_SUM_BLKADDR(sbi, segno);
	ret = dev_write_block(sum_blk, ssa_blkaddr, WRITE_LIFE_NONE);
	ASSERT(ret >= 0);

out:
	if (type == SEG_TYPE_NODE || type == SEG_TYPE_DATA ||
	    type == SEG_TYPE_MAX)
		free(sum_blk);
	return ret;
}

static int inject_inode(struct f2fs_sb_info *sbi, struct f2fs_node *node,
			struct inject_option *opt)
{
	struct f2fs_inode *inode = &node->i;

	if (!strcmp(opt->mb, "i_mode")) {
		MSG(0, "Info: inject inode i_mode of nid %u: 0x%x -> 0x%x\n",
		    opt->nid, le16_to_cpu(inode->i_mode), (u16)opt->val);
		inode->i_mode = cpu_to_le16((u16)opt->val);
	} else if (!strcmp(opt->mb, "i_advise")) {
		MSG(0, "Info: inject inode i_advise of nid %u: 0x%x -> 0x%x\n",
		    opt->nid, inode->i_advise, (u8)opt->val);
		inode->i_advise = (u8)opt->val;
	} else if (!strcmp(opt->mb, "i_inline")) {
		MSG(0, "Info: inject inode i_inline of nid %u: 0x%x -> 0x%x\n",
		    opt->nid, inode->i_inline, (u8)opt->val);
		inode->i_inline = (u8)opt->val;
	} else if (!strcmp(opt->mb, "i_links")) {
		MSG(0, "Info: inject inode i_links of nid %u: %u -> %u\n",
		    opt->nid, le32_to_cpu(inode->i_links), (u32)opt->val);
		inode->i_links = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "i_size")) {
		MSG(0, "Info: inject inode i_size of nid %u: %"PRIu64" -> %"PRIu64"\n",
		    opt->nid, le64_to_cpu(inode->i_size), (u64)opt->val);
		inode->i_size = cpu_to_le64((u64)opt->val);
	} else if (!strcmp(opt->mb, "i_blocks")) {
		MSG(0, "Info: inject inode i_blocks of nid %u: %"PRIu64" -> %"PRIu64"\n",
		    opt->nid, le64_to_cpu(inode->i_blocks), (u64)opt->val);
		inode->i_blocks = cpu_to_le64((u64)opt->val);
	} else if (!strcmp(opt->mb, "i_xattr_nid")) {
		MSG(0, "Info: inject inode i_xattr_nid of nid %u: %u -> %u\n",
		    opt->nid, le32_to_cpu(inode->i_xattr_nid), (u32)opt->val);
		inode->i_xattr_nid = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "i_ext.fofs")) {
		MSG(0, "Info: inject inode i_ext.fofs of nid %u: %u -> %u\n",
		    opt->nid, le32_to_cpu(inode->i_ext.fofs), (u32)opt->val);
		inode->i_ext.fofs = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "i_ext.blk_addr")) {
		MSG(0, "Info: inject inode i_ext.blk_addr of nid %u: "
		    "0x%x -> 0x%x\n", opt->nid,
		    le32_to_cpu(inode->i_ext.blk_addr), (u32)opt->val);
		inode->i_ext.blk_addr = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "i_ext.len")) {
		MSG(0, "Info: inject inode i_ext.len of nid %u: %u -> %u\n",
		    opt->nid, le32_to_cpu(inode->i_ext.len), (u32)opt->val);
		inode->i_ext.len = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "i_extra_isize")) {
		/* do not care if F2FS_EXTRA_ATTR is enabled */
		MSG(0, "Info: inject inode i_extra_isize of nid %u: %d -> %d\n",
		    opt->nid, le16_to_cpu(inode->i_extra_isize), (u16)opt->val);
		inode->i_extra_isize = cpu_to_le16((u16)opt->val);
	} else if (!strcmp(opt->mb, "i_inline_xattr_size")) {
		MSG(0, "Info: inject inode i_inline_xattr_size of nid %u: "
		    "%d -> %d\n", opt->nid,
		    le16_to_cpu(inode->i_inline_xattr_size), (u16)opt->val);
		inode->i_inline_xattr_size = cpu_to_le16((u16)opt->val);
	} else if (!strcmp(opt->mb, "i_inode_checksum")) {
		MSG(0, "Info: inject inode i_inode_checksum of nid %u: "
		    "0x%x -> 0x%x\n", opt->nid,
		    le32_to_cpu(inode->i_inode_checksum), (u32)opt->val);
		inode->i_inode_checksum = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "i_compr_blocks")) {
		MSG(0, "Info: inject inode i_compr_blocks of nid %u: "
		    "%"PRIu64" -> %"PRIu64"\n", opt->nid,
		    le64_to_cpu(inode->i_compr_blocks), (u64)opt->val);
		inode->i_compr_blocks = cpu_to_le64((u64)opt->val);
	} else if (!strcmp(opt->mb, "i_addr")) {
		/* do not care if it is inline data */
		if (opt->idx >= DEF_ADDRS_PER_INODE) {
			ERR_MSG("invalid index %u of i_addr[]\n", opt->idx);
			return -EINVAL;
		}
		MSG(0, "Info: inject inode i_addr[%d] of nid %u: "
		    "0x%x -> 0x%x\n", opt->idx, opt->nid,
		    le32_to_cpu(inode->i_addr[opt->idx]), (u32)opt->val);
		inode->i_addr[opt->idx] = cpu_to_le32((block_t)opt->val);
	} else if (!strcmp(opt->mb, "i_nid")) {
		if (opt->idx >= 5) {
			ERR_MSG("invalid index %u of i_nid[]\n", opt->idx);
			return -EINVAL;
		}
		MSG(0, "Info: inject inode i_nid[%d] of nid %u: "
		    "0x%x -> 0x%x\n", opt->idx, opt->nid,
		    le32_to_cpu(F2FS_INODE_I_NID(inode, opt->idx)),
		    (u32)opt->val);
		F2FS_INODE_I_NID(inode, opt->idx) = cpu_to_le32((nid_t)opt->val);
	} else {
		ERR_MSG("unknown or unsupported member \"%s\"\n", opt->mb);
		return -EINVAL;
	}

	if (c.dbg_lv > 0)
		print_node_info(sbi, node, 1);

	return 0;
}

static int inject_index_node(struct f2fs_sb_info *sbi, struct f2fs_node *node,
			     struct inject_option *opt)
{
	struct direct_node *dn = &node->dn;

	if (strcmp(opt->mb, "addr")) {
		ERR_MSG("unknown or unsupported member \"%s\"\n", opt->mb);
		return -EINVAL;
	}

	if (opt->idx >= DEF_ADDRS_PER_BLOCK) {
		ERR_MSG("invalid index %u of nid/addr[]\n", opt->idx);
		return -EINVAL;
	}

	MSG(0, "Info: inject node nid/addr[%d] of nid %u: 0x%x -> 0x%x\n",
	    opt->idx, opt->nid, le32_to_cpu(dn->addr[opt->idx]),
	    (block_t)opt->val);
	dn->addr[opt->idx] = cpu_to_le32((block_t)opt->val);

	if (c.dbg_lv > 0)
		print_node_info(sbi, node, 1);

	return 0;
}

static int inject_node(struct f2fs_sb_info *sbi, struct inject_option *opt)
{
	struct f2fs_super_block *sb = sbi->raw_super;
	struct node_info ni;
	struct f2fs_node *node_blk;
	struct node_footer *footer;
	int ret;

	if (!IS_VALID_NID(sbi, opt->nid)) {
		ERR_MSG("Invalid nid %u range [%u:%"PRIu64"]\n", opt->nid, 0,
			NAT_ENTRY_PER_BLOCK *
			((get_sb(segment_count_nat) << 1) <<
			 sbi->log_blocks_per_seg));
		return -EINVAL;
	}

	node_blk = calloc(F2FS_BLKSIZE, 1);
	ASSERT(node_blk);

	get_node_info(sbi, opt->nid, &ni);
	ret = dev_read_block(node_blk, ni.blk_addr);
	ASSERT(ret >= 0);
	footer = F2FS_NODE_FOOTER(node_blk);

	if (!strcmp(opt->mb, "nid")) {
		MSG(0, "Info: inject node footer nid of nid %u: %u -> %u\n",
		    opt->nid, le32_to_cpu(footer->nid), (u32)opt->val);
		footer->nid = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "ino")) {
		MSG(0, "Info: inject node footer ino of nid %u: %u -> %u\n",
		    opt->nid, le32_to_cpu(footer->ino), (u32)opt->val);
		footer->ino = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "flag")) {
		MSG(0, "Info: inject node footer flag of nid %u: "
		    "0x%x -> 0x%x\n", opt->nid, le32_to_cpu(footer->flag),
		    (u32)opt->val);
		footer->flag = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "cp_ver")) {
		MSG(0, "Info: inject node footer cp_ver of nid %u: "
		    "0x%"PRIx64" -> 0x%"PRIx64"\n", opt->nid, le64_to_cpu(footer->cp_ver),
		    (u64)opt->val);
		footer->cp_ver = cpu_to_le64((u64)opt->val);
	} else if (!strcmp(opt->mb, "next_blkaddr")) {
		MSG(0, "Info: inject node footer next_blkaddr of nid %u: "
		    "0x%x -> 0x%x\n", opt->nid,
		    le32_to_cpu(footer->next_blkaddr), (u32)opt->val);
		footer->next_blkaddr = cpu_to_le32((u32)opt->val);
	} else if (ni.nid == ni.ino) {
		ret = inject_inode(sbi, node_blk, opt);
	} else {
		ret = inject_index_node(sbi, node_blk, opt);
	}
	if (ret)
		goto out;

	print_node_footer_info(footer);

	/*
	 * if i_inode_checksum is injected, should call update_block() to
	 * avoid recalculate inode checksum
	 */
	if (ni.nid == ni.ino && strcmp(opt->mb, "i_inode_checksum"))
		ret = update_inode(sbi, node_blk, &ni.blk_addr);
	else
		ret = update_block(sbi, node_blk, &ni.blk_addr, NULL);
	ASSERT(ret >= 0);

out:
	free(node_blk);
	return ret;
}

static int find_dir_entry(struct f2fs_dentry_ptr *d, nid_t ino)
{
	struct f2fs_dir_entry *de;
	int slot = 0;

	while (slot < d->max) {
		if (!test_bit_le(slot, d->bitmap)) {
			slot++;
			continue;
		}

		de = &d->dentry[slot];
		if (de->name_len == 0) {
			slot++;
			continue;
		}
		if (le32_to_cpu(de->ino) == ino)
			return slot;
		slot += GET_DENTRY_SLOTS(le16_to_cpu(de->name_len));
	}

	return -ENOENT;
}

static int inject_dentry(struct f2fs_sb_info *sbi, struct inject_option *opt)
{
	struct node_info ni;
	struct f2fs_node *node_blk = NULL;
	struct f2fs_inode *inode;
	struct f2fs_dentry_ptr d;
	void *buf = NULL, *inline_dentry;
	struct f2fs_dentry_block *dent_blk = NULL;
	block_t addr = 0;
	struct f2fs_dir_entry *dent = NULL;
	struct dnode_of_data dn;
	nid_t pino;
	int slot = -ENOENT, namelen, namecap, ret;
	unsigned int dentry_hash;
	char *name;

	node_blk = malloc(F2FS_BLKSIZE);
	ASSERT(node_blk != NULL);

	/* get child inode */
	get_node_info(sbi, opt->nid, &ni);
	ret = dev_read_block(node_blk, ni.blk_addr);
	ASSERT(ret >= 0);

	if (opt->dots) {
		if (!LINUX_S_ISDIR(le16_to_cpu(node_blk->i.i_mode))) {
			ERR_MSG("ino %u is not a directory, cannot inject "
				"its %s\n", opt->nid,
				opt->dots == TYPE_DOT ? "." : "..");
			ret = -EINVAL;
			goto out;
		}
		/* pino is itself */
		pino = opt->nid;
	} else {
		pino = le32_to_cpu(node_blk->i.i_pino);

		/* get parent inode */
		get_node_info(sbi, pino, &ni);
		ret = dev_read_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
	}
	inode = &node_blk->i;

	/* find child dentry */
	if (inode->i_inline & F2FS_INLINE_DENTRY) {
		inline_dentry = inline_data_addr(node_blk);
		make_dentry_ptr(&d, node_blk, inline_dentry, 2);
		addr = ni.blk_addr;
		buf = node_blk;

		if (opt->dots == TYPE_DOTDOT)
			slot = find_dir_entry(&d, le32_to_cpu(node_blk->i.i_pino));
		else
			slot = find_dir_entry(&d, opt->nid);
		if (slot >= 0)
			dent = &d.dentry[slot];
	} else {
		unsigned int level, dirlevel, nbucket;
		unsigned long i, end;

		level = le32_to_cpu(inode->i_current_depth);
		dirlevel = le32_to_cpu(inode->i_dir_level);
		nbucket = dir_buckets(level, dirlevel);
		end = dir_block_index(level, dirlevel, nbucket) +
						bucket_blocks(level);

		dent_blk = malloc(F2FS_BLKSIZE);
		ASSERT(dent_blk != NULL);

		for (i = 0; i < end; i++) {
			memset(&dn, 0, sizeof(dn));
			set_new_dnode(&dn, node_blk, NULL, pino);
			ret = get_dnode_of_data(sbi, &dn, i, LOOKUP_NODE);
			if (ret < 0)
				break;
			addr = dn.data_blkaddr;
			if (dn.inode_blk != dn.node_blk)
				free(dn.node_blk);
			if (addr == NULL_ADDR || addr == NEW_ADDR)
				continue;
			if (!f2fs_is_valid_blkaddr(sbi, addr, DATA_GENERIC)) {
				MSG(0, "invalid blkaddr 0x%x at offset %lu\n",
				    addr, i);
				continue;
			}
			ret = dev_read_block(dent_blk, addr);
			ASSERT(ret >= 0);

			make_dentry_ptr(&d, node_blk, dent_blk, 1);
			if (opt->dots == TYPE_DOTDOT)
				slot = find_dir_entry(&d, le32_to_cpu(node_blk->i.i_pino));
			else
				slot = find_dir_entry(&d, opt->nid);
			if (slot >= 0) {
				dent = &d.dentry[slot];
				buf = dent_blk;
				break;
			}
		}
	}

	if (slot < 0) {
		ERR_MSG("dentry of ino %u not found\n", opt->nid);
		ret = -ENOENT;
		goto out;
	}

	if (!strcmp(opt->mb, "d_bitmap")) {
		MSG(0, "Info: inject dentry bitmap of nid %u: 1 -> 0\n",
		    opt->nid);
		test_and_clear_bit_le(slot, d.bitmap);
	} else if (!strcmp(opt->mb, "d_hash")) {
		MSG(0, "Info: inject dentry d_hash of nid %u: "
		    "0x%x -> 0x%x\n", opt->nid, le32_to_cpu(dent->hash_code),
		    (u32)opt->val);
		dent->hash_code = cpu_to_le32((u32)opt->val);
	} else if (!strcmp(opt->mb, "d_ino")) {
		MSG(0, "Info: inject dentry d_ino of nid %u: "
		    "%u -> %u\n", opt->nid, le32_to_cpu(dent->ino),
		    (nid_t)opt->val);
		dent->ino = cpu_to_le32((nid_t)opt->val);
	} else if (!strcmp(opt->mb, "d_ftype")) {
		MSG(0, "Info: inject dentry d_type of nid %u: "
		    "%d -> %d\n", opt->nid, dent->file_type,
		    (u8)opt->val);
		dent->file_type = (u8)opt->val;
	} else if (!strcmp(opt->mb, "filename")) {
		if (!opt->str) {
			ERR_MSG("option str is needed\n");
			ret = -EINVAL;
			goto out;
		}
		namecap = ALIGN_UP(le16_to_cpu(dent->name_len), F2FS_SLOT_LEN);
		namelen = strlen(opt->str);
		if (namelen > namecap || namelen > F2FS_NAME_LEN) {
			ERR_MSG("option str too long\n");
			ret = -EINVAL;
			goto out;
		}
		name = (char *)d.filename[slot];
		MSG(0, "Info: inject dentry filename of nid %u: "
		    "%.*s -> %s\n", opt->nid, le16_to_cpu(dent->name_len),
		    name, opt->str);
		memcpy(name, opt->str, namelen);
		MSG(0, "Info: inject dentry namelen of nid %u: "
		    "%d -> %d\n", opt->nid, le16_to_cpu(dent->name_len),
		    namelen);
		dent->name_len = cpu_to_le16(namelen);
		dentry_hash = f2fs_dentry_hash(get_encoding(sbi),
						IS_CASEFOLDED(inode),
						(unsigned char *)name,
						namelen);
		MSG(0, "Info: inject dentry d_hash of nid %u: "
		    "0x%x -> 0x%x\n", opt->nid, le32_to_cpu(dent->hash_code),
		    dentry_hash);
		dent->hash_code = cpu_to_le32(dentry_hash);
	} else {
		ERR_MSG("unknown or unsupported member \"%s\"\n", opt->mb);
		ret = -EINVAL;
		goto out;
	}

	print_raw_dentry_info(dent);

	if (inode->i_inline & F2FS_INLINE_DENTRY)
		ret = update_inode(sbi, buf, &addr);
	else
		ret = update_block(sbi, buf, &addr, NULL);
	ASSERT(ret >= 0);

out:
	free(node_blk);
	free(dent_blk);
	return ret;
}

int do_inject(struct f2fs_sb_info *sbi)
{
	struct inject_option *opt = (struct inject_option *)c.private;
	int ret = -EINVAL;

	if (c.zoned_model == F2FS_ZONED_HM)
		fsck_init(sbi);

	if (opt->sb >= 0)
		ret = inject_sb(sbi, opt);
	else if (opt->cp >= 0)
		ret = inject_cp(sbi, opt);
	else if (opt->nat >= 0)
		ret = inject_nat(sbi, opt);
	else if (opt->sit >= 0)
		ret = inject_sit(sbi, opt);
	else if (opt->ssa)
		ret = inject_ssa(sbi, opt);
	else if (opt->node)
		ret = inject_node(sbi, opt);
	else if (opt->dent)
		ret = inject_dentry(sbi, opt);

	if (c.zoned_model == F2FS_ZONED_HM) {
		if (!ret && (opt->node || opt->dent)) {
			write_curseg_info(sbi);
			flush_journal_entries(sbi);
			flush_sit_entries(sbi);
			write_checkpoint(sbi);
		}
		fsck_free(sbi);
	}

	return ret;
}
