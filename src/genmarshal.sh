#!/bin/bash
glib-genmarshal --header --prefix ol_marshal marshal > ol_marshal.h
glib-genmarshal --body --prefix ol_marshal marshal > ol_marshal.c

