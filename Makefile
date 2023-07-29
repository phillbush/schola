PROG = simp
SRCS = simp.c data.c port.c eval.c gc.c io.c
OBJS = ${SRCS:.c=.o} stdlib.o
MANS = simp.1 simp.7
HEDS = simp.h

DEFS = -D_POSIX_C_SOURCE=200809L
LIBS = -lm

PDFS = simp.7.pdf simp.1.pdf

LDEMULATION ?= elf_x86_64

all: ${PROG}

${PROG}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS} ${LDFLAGS}

.c.o:
	${CC} -std=c99 -pedantic ${DEFS} ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

stdlib.o: stdlib.lisp
	${LD} -r -b binary -o stdlib.o -m ${LDEMULATION} stdlib.lisp

${PDFS}: ${@:.pdf=}
	{ printf '.fp 5 CW DejaVuSansMono\n' ; cat "${@:.pdf=}" ; } | \
	eqn | tbl | troff -mdoc - | dpost | ps2pdf -sPAPERSIZE=letter - >"$@"

${OBJS}: ${HEDS}

tags: ${SRCS}
	ctags ${SRCS}

lint: ${SRCS}
	-mandoc -T lint -W warning ${MANS}
	-clang-tidy ${SRCS} -- -std=c99 ${DEFS} ${CPPFLAGS}

loc:
	cat ${SRCS} ${HEDS} | egrep -v '^([[:blank:]]|/\*.*\*/)*$$' | wc -l

clean:
	rm -f ${OBJS} ${PROG} ${PROG:=.core} tags

.PHONY: all clean lint loc
