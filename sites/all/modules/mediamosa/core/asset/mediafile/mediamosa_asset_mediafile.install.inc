<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * The install file for asset media
 */

require_once 'mediamosa_asset_mediafile.class.inc';
require_once 'mediamosa_asset_mediafile_db.class.inc';
require_once 'metadata/mediamosa_asset_mediafile_metadata.install.inc';
require_once 'still/mediamosa_asset_mediafile_still.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_mediafile_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_mediafile_db::TABLE_NAME);

  // Call the asset media metadata installer.
  _mediamosa_asset_mediafile_metadata_install();

  // Call the asset media still installer.
  _mediamosa_asset_mediafile_still_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_mediafile_uninstall() {
  // Call the asset media still uninstaller.
  _mediamosa_asset_mediafile_still_uninstall();

  // Call the asset media metadata uninstaller.
  _mediamosa_asset_mediafile_metadata_uninstall();

  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_mediafile_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_mediafile_schema() {

  $schema[mediamosa_asset_mediafile_db::TABLE_NAME] = array(
  'description' => 'The asset table.',
    'fields' => array(
      mediamosa_asset_mediafile_db::ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_asset_mediafile_db::ASSET_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The parent asset ID.',
      ),
      mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'default' => NULL,
        'description' => 'The source or original ID of the media from which this media was created.',
      ),
      mediamosa_asset_mediafile_db::TAG => array(
        'type' => 'varbinary',
        'length' => mediamosa_asset_mediafile_db::TAG_LENGTH,
        'default' => NULL,
        'description' => 'Can contain user defined tag.',
      ),
      mediamosa_asset_mediafile_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the asset.',
      ),
      mediamosa_asset_mediafile_db::OWNER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_db::OWNER_ID_LENGTH,
        'default' => NULL,
        'description' => 'The owner ID.',
      ),
      mediamosa_asset_mediafile_db::GROUP_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_db::GROUP_ID_LENGTH,
        'default' => NULL,
        'description' => 'The group ID.',
      ),
      mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => array(
        'description' => 'This is the original media.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE)) . "')",
        'default' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE,
      ),
      mediamosa_asset_mediafile_db::IS_DOWNLOADABLE => array(
        'description' => 'The media can be downloaded.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_FALSE, mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_FALSE, mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_TRUE)) . "')",
        'default' => mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_FALSE,
      ),
      mediamosa_asset_mediafile_db::IS_STREAMABLE => array(
        'description' => 'The media can be streamed.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_STREAMABLE_FALSE, mediamosa_asset_mediafile_db::IS_STREAMABLE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_STREAMABLE_FALSE, mediamosa_asset_mediafile_db::IS_STREAMABLE_TRUE)) . "')",
        'default' => mediamosa_asset_mediafile_db::IS_STREAMABLE_FALSE,
      ),
      mediamosa_asset_mediafile_db::IS_STILL => array(
        'description' => 'The media is an still.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_STILL_FALSE, mediamosa_asset_mediafile_db::IS_STILL_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_STILL_FALSE, mediamosa_asset_mediafile_db::IS_STILL_TRUE)) . "')",
        'default' => mediamosa_asset_mediafile_db::IS_STILL_FALSE,
      ),
      mediamosa_asset_mediafile_db::FILENAME => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_db::FILENAME_LENGTH,
        'default' => NULL,
        'description' => 'The optional filename.',
      ),
      mediamosa_asset_mediafile_db::URI => array(
        'type' => 'text',
        'length' => mediamosa_asset_mediafile_db::URI_LENGTH,
        'default' => NULL,
        'description' => 'The optional URI location.',
      ),
      mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => array(
        'type' => 'text',
        'default' => NULL,
        'description' => 'The san-nas mount location of the file.',
      ),
      mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'description' => 'The transcoding profile ID.',
      ),
      mediamosa_asset_mediafile_db::TOOL => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_db::TOOL_LENGTH,
        'default' => NULL,
        'description' => 'The tool used when creating file.',
      ),
      mediamosa_asset_mediafile_db::COMMAND => array(
        'type' => 'text',
        'default' => NULL,
        'description' => 'The command used to create the file.',
      ),
      mediamosa_asset_mediafile_db::FILE_EXTENSION => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_db::FILE_EXTENSION_LENGTH,
        'default' => NULL,
        'description' => 'The file extension.',
      ),
      mediamosa_asset_mediafile_db::IS_PROTECTED => array(
        'description' => 'Set kind of protection on the media.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE, mediamosa_asset_mediafile_db::IS_PROTECTED_USER_USERGROUP, mediamosa_asset_mediafile_db::IS_PROTECTED_DOMAIN_REALM, mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE, mediamosa_asset_mediafile_db::IS_PROTECTED_USER_USERGROUP, mediamosa_asset_mediafile_db::IS_PROTECTED_DOMAIN_REALM, mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE)) . "')",
        'default' => mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE,
      ),
      mediamosa_asset_mediafile_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when media was created.',
      ),
      mediamosa_asset_mediafile_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when media was changed last.',
      ),
      mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL => array(
        'description' => 'The asset can be locked or unlocked.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_FALSE, mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_FALSE, mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_TRUE)) . "')",
        'default' => mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_TRUE,
      ),
    ),
    'primary key' => array(mediamosa_asset_mediafile_db::ID),
    'indexes' => array(
      'idx_' . mediamosa_asset_mediafile_db::OWNER_ID => array(mediamosa_asset_mediafile_db::OWNER_ID),
      'idx_' . mediamosa_asset_mediafile_db::GROUP_ID => array(mediamosa_asset_mediafile_db::GROUP_ID),
      'idx_' . mediamosa_asset_mediafile_db::ASSET_ID => array(mediamosa_asset_mediafile_db::ASSET_ID),
      'idx_appid_mediafileid' => array(mediamosa_asset_mediafile_db::APP_ID, mediamosa_asset_mediafile_db::ID),
      'idx_appid_original_protected' => array(mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::APP_ID, mediamosa_asset_mediafile_db::IS_PROTECTED),
      'idx_mediafileid_appid_original_protected' => array(mediamosa_asset_mediafile_db::ID, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::APP_ID, mediamosa_asset_mediafile_db::IS_PROTECTED),
      'idx_filename' => array(mediamosa_asset_mediafile_db::FILENAME),
      'idx_protected' => array(mediamosa_asset_mediafile_db::IS_PROTECTED),
    ),
    'foreign keys' => array(
      mediamosa_asset_mediafile_db::ASSET_ID => array(mediamosa_asset_db::TABLE_NAME => mediamosa_asset_db::ID),
    ),
  );

  return $schema;
}
