
#ifndef _CLIENT_H_
#define _CLIENT_H_

int connect_to_collectd(const char *address);
int get_single_value_from_collectd(const char *name, double *val);
int check_stat_names(void);
int get_list_of_indexes(snmp_table_t *table,
						uint32_t    **ret_indexes, 
						size_t       *ret_num_indexes);

#endif /* _CLIENT_H */
