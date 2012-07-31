bin_PROGRAMS = collectd-agent
collectd_agent_SOURCES = main.c parser.y scanner.l agent.c client.c \
			 agent_helper.c common.c agent.h  agent_helper.h  \
			 client.h  common.h  data.h  settings.h

BUILT_SOURCES = parser.h
AM_YFLAGS = -d
CLEANFILES = parser.[ch] scanner.c

AM_LDFLAGS = $(SNMP_LIBS)

