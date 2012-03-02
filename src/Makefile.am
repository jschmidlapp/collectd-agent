bin_PROGRAMS = collectd-agent
collectd_agent_SOURCES = main.c parser.y scanner.l agent.c client.c agent_helper.c common.c

BUILT_SOURCES = parser.h
AM_YFLAGS = -d
CLEANFILES = parser.[ch] scanner.c

AM_LDFLAGS = $(SNMP_LIBS)

