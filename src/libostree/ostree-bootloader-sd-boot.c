/*
 * Copyright (C) 2022 Foundries.IO Ltd
 *
 * Based on ot-bootloader-uboot.c by Colin Walters <walters@verbum.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the licence or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"

#include "ostree-sysroot-private.h"
#include "ostree-bootloader-sd-boot.h"
#include "otutil.h"

#include <string.h>

static const char sdboot_config_path[] = "boot/loader/loader.conf";

struct _OstreeBootloaderSdboot
{
  GObject       parent_instance;

  OstreeSysroot  *sysroot;
};

typedef GObjectClass OstreeBootloaderSdbootClass;

static void _ostree_bootloader_sdboot_bootloader_iface_init (OstreeBootloaderInterface *iface);
G_DEFINE_TYPE_WITH_CODE (OstreeBootloaderSdboot, _ostree_bootloader_sdboot, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (OSTREE_TYPE_BOOTLOADER, _ostree_bootloader_sdboot_bootloader_iface_init));

static gboolean
_ostree_bootloader_sdboot_query (OstreeBootloader *bootloader,
                                gboolean         *out_is_active,
                                GCancellable     *cancellable,
                                GError          **error)
{
  OstreeBootloaderSdboot *self = OSTREE_BOOTLOADER_SDBOOT (bootloader);
  struct stat stbuf;

  if (!glnx_fstatat_allow_noent (self->sysroot->sysroot_fd, sdboot_config_path, &stbuf, AT_SYMLINK_NOFOLLOW, error))
    return FALSE;
  *out_is_active = (errno == 0);
  return TRUE;
}

static const char *
_ostree_bootloader_sdboot_get_name (OstreeBootloader *bootloader)
{
  return "systemd-boot";
}

static gboolean
_ostree_bootloader_sdboot_write_config (OstreeBootloader *bootloader,
                                       int               new_bootversion,
                                       GPtrArray        *new_deployments,
                                       GCancellable     *cancellable,
                                       GError          **error)
{
  OstreeBootloaderSdboot *self = OSTREE_BOOTLOADER_SDBOOT (bootloader);

  /* This should follow the symbolic link to the new bootversion. */
  g_autofree char *config_contents =
    glnx_file_get_contents_utf8_at (self->sysroot->sysroot_fd, sdboot_config_path, NULL,
                                    cancellable, error);
  if (!config_contents)
    return FALSE;

  g_autofree char *new_config_path = g_strdup_printf ("boot/loader.%d/loader.conf", new_bootversion);
  if (!glnx_file_replace_contents_at (self->sysroot->sysroot_fd, new_config_path,
                                      (guint8*)config_contents, strlen (config_contents),
                                      GLNX_FILE_REPLACE_DATASYNC_NEW,
                                      cancellable, error))
    return FALSE;

  return TRUE;
}

static void
_ostree_bootloader_sdboot_finalize (GObject *object)
{
  OstreeBootloaderSdboot *self = OSTREE_BOOTLOADER_SDBOOT (object);

  g_clear_object (&self->sysroot);

  G_OBJECT_CLASS (_ostree_bootloader_sdboot_parent_class)->finalize (object);
}

void
_ostree_bootloader_sdboot_init (OstreeBootloaderSdboot *self)
{
}

static void
_ostree_bootloader_sdboot_bootloader_iface_init (OstreeBootloaderInterface *iface)
{
  iface->query = _ostree_bootloader_sdboot_query;
  iface->get_name = _ostree_bootloader_sdboot_get_name;
  iface->write_config = _ostree_bootloader_sdboot_write_config;
}

void
_ostree_bootloader_sdboot_class_init (OstreeBootloaderSdbootClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = _ostree_bootloader_sdboot_finalize;
}

OstreeBootloaderSdboot *
_ostree_bootloader_sdboot_new (OstreeSysroot *sysroot)
{
  OstreeBootloaderSdboot *self = g_object_new (OSTREE_TYPE_BOOTLOADER_SDBOOT, NULL);
  self->sysroot = g_object_ref (sysroot);
  return self;
}
