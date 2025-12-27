/*
 * tsinfo.c - netlink implementation of timestamping commands
 *
 * Implementation of "ethtool -T <dev>"
 */

#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "bitset.h"

/* TSINFO_GET */

static int tsinfo_show_stats(const struct nlattr *nest)
{
	const struct nlattr *tb[ETHTOOL_A_TS_STAT_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	static const struct {
		unsigned int attr;
		char *name;
	} stats[] = {
		{ ETHTOOL_A_TS_STAT_TX_PKTS, "tx_pkts" },
		{ ETHTOOL_A_TS_STAT_TX_LOST, "tx_lost" },
		{ ETHTOOL_A_TS_STAT_TX_ERR, "tx_err" },
	};
	bool header = false;
	unsigned int i;
	__u64 val;
	int ret;

	ret = mnl_attr_parse_nested(nest, attr_cb, &tb_info);
	if (ret < 0)
		return ret;

	open_json_object("statistics");
	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		char fmt[64];

		if (!tb[stats[i].attr])
			continue;

		if (!header && !is_json_context()) {
			printf("Statistics:\n");
			header = true;
		}

		if (!mnl_attr_validate(tb[stats[i].attr], MNL_TYPE_U32)) {
			val = mnl_attr_get_u32(tb[stats[i].attr]);
		} else if (!mnl_attr_validate(tb[stats[i].attr], MNL_TYPE_U64)) {
			val = mnl_attr_get_u64(tb[stats[i].attr]);
		} else {
			fprintf(stderr, "malformed netlink message (statistic)\n");
			goto err_close_stats;
		}

		snprintf(fmt, sizeof(fmt), "  %s: %%" PRIu64 "\n", stats[i].name);
		print_u64(PRINT_ANY, stats[i].name, fmt, val);
	}
	close_json_object();

	return 0;

err_close_stats:
	close_json_object();
	return -1;
}

static void tsinfo_dump_cb(unsigned int idx, const char *name, bool val,
			   void *data __maybe_unused)
{
	if (!val)
		return;

	if (name)
		printf("\t%s\n", name);
	else
		printf("\tbit%u\n", idx);
}

static int tsinfo_dump_list(struct nl_context *nlctx, const struct nlattr *attr,
			    const char *label, const char *if_empty,
			    unsigned int stringset_id)
{
	const struct stringset *strings = NULL;
	int ret;

	printf("%s:", label);
	ret = 0;
	if (!attr || bitset_is_empty(attr, false, &ret)) {
		printf("%s\n", if_empty);
		return ret;
	}
	putchar('\n');
	if (ret < 0)
		return ret;

	if (bitset_is_compact(attr)) {
		ret = netlink_init_ethnl2_socket(nlctx);
		if (ret < 0)
			return ret;
		strings = global_stringset(stringset_id, nlctx->ethnl2_socket);
	}
	return walk_bitset(attr, strings, tsinfo_dump_cb, NULL);
}

int tsinfo_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_TSINFO_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_TSINFO_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		putchar('\n');
	printf("Time stamping parameters for %s:\n", nlctx->devname);

	ret = tsinfo_dump_list(nlctx, tb[ETHTOOL_A_TSINFO_TIMESTAMPING],
			       "Capabilities", "", ETH_SS_SOF_TIMESTAMPING);
	if (ret < 0)
		return err_ret;

	printf("PTP Hardware Clock: ");
	if (tb[ETHTOOL_A_TSINFO_PHC_INDEX])
		printf("%d\n",
		       mnl_attr_get_u32(tb[ETHTOOL_A_TSINFO_PHC_INDEX]));
	else
		printf("none\n");

	ret = tsinfo_dump_list(nlctx, tb[ETHTOOL_A_TSINFO_TX_TYPES],
			       "Hardware Transmit Timestamp Modes", " none",
			       ETH_SS_TS_TX_TYPES);
	if (ret < 0)
		return err_ret;

	ret = tsinfo_dump_list(nlctx, tb[ETHTOOL_A_TSINFO_RX_FILTERS],
			       "Hardware Receive Filter Modes", " none",
			       ETH_SS_TS_RX_FILTERS);
	if (ret < 0)
		return err_ret;

	if (tb[ETHTOOL_A_TSINFO_STATS]) {
		ret = tsinfo_show_stats(tb[ETHTOOL_A_TSINFO_STATS]);
		if (ret < 0)
			return err_ret;
	}

	return MNL_CB_OK;
}

int nl_tsinfo(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	u32 flags;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_TSINFO_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	flags = get_stats_flag(nlctx, ETHTOOL_MSG_TSINFO_GET, ETHTOOL_A_TSINFO_HEADER);
	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_TSINFO_GET,
				      ETHTOOL_A_TSINFO_HEADER, flags);
	if (ret < 0)
		return ret;
	return nlsock_send_get_request(nlsk, tsinfo_reply_cb);
}
