CC=	arm-elf-gcc
CFLAGS=	-Os -fno-builtin -mthumb-interwork -mthumb
ASFLAGS=-mthumb-interwork
LD=	arm-elf-ld

OBJS=	cmd_dump.o cmd_r16.o cmd_r32.o cmd_r8.o cmd_w16.o cmd_w32.o cmd_w8.o \
	cmdentry.o cmdtab.o dispatch.o doprnt.o entry.o entryinfo.o getchar.o \
	hexarg.o main.o parseargs.o printf.o putchar.o serio.o

all:	xipcode.o

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *errs
