#	$NetBSD: Makefile,v 1.4 2003/09/30 09:49:29 lukem Exp $

.include "../Makefile.serverlib"

.include <bsd.own.mk>

LIB=		lbx
.PATH:		${X11SRCDIR.xc}/programs/Xserver/lbx

SRCS=		lbxcmap.c lbxdix.c lbxexts.c lbxgfx.c lbxmain.c lbxopts.c \
		lbxprop.c lbxsquish.c lbxswap.c lbxtables.c lbxtags.c \
		lbxzerorep.c

CPPFLAGS+=	-I${DESTDIR}${X11INCDIR}/X11 \
		-I${X11SRCDIR.xc}/include/fonts \
		-I${X11SRCDIR.xc}/programs/Xserver/include \
		-I${DESTDIR}${X11INCDIR}/X11/extensions

COPTS.lbxgfx.c=	-Wno-error	# XXX gcc 3.3

.include <bsd.x11.mk>
.include <bsd.lib.mk>
