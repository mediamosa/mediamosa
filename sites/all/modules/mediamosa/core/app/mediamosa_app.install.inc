<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * Installer for the App module.
  */

require_once 'mediamosa_app.class.inc';
require_once 'mediamosa_app_db.class.inc';
require_once 'oauth/mediamosa_app_oauth_nonce.install.inc';
require_once 'oauth/mediamosa_app_oauth_token.install.inc';

/**
 * Not hook_install().
 */
function _mediamosa_app_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_app_db::TABLE_NAME);

  _mediamosa_app_oauth_nonce_install();
  _mediamosa_app_oauth_token_install();
}

/**
 * Not hook_uninstall().
 */
function _mediamosa_app_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_app_db::TABLE_NAME);

  _mediamosa_app_oauth_nonce_uninstall();
  _mediamosa_app_oauth_token_uninstall();
}

/**
 * Implements hook_schema().
 * @return array
 */
function mediamosa_app_schema() {
  $schema[mediamosa_app_db::TABLE_NAME] = array(
    'description' => 'The client applications are stored here. Each app is one client application.',
    'fields' => array(
      mediamosa_app_db::NID => array(
        'description' => 'The Drupal node id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_app_db::VID => array(
        'description' => 'The Drupal node revision id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_app_db::APP_NAME => array(
        'description' => 'The name of the client.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::APP_NAME_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_app_db::APP_ID => array(
        'description' => 'The unique application ID.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      mediamosa_app_db::TRANSCODING_POSITION_FROM => array(
        'description' => 'Starting range of allowed transcoding.',
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_db::TRANSCODING_POSITION_TO => array(
        'description' => 'Ending range of allowed transcoding.',
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_db::QUOTA => array(
        'description' => 'The total allowed quota in gigabytes.',
        'type' => 'int',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_db::QUOTA_USED => array(
        'description' => 'The current usage in gigabytes.',
        'type' => 'int',
        'unsigned' => TRUE,
        'size' => 'big',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_db::SHARED_KEY => array(
        'description' => 'The key to identify the client.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::SHARED_KEY_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ACTIVE => array(
        'description' => 'The client can be enabled or disabled.',
        'type' => "enum('" . implode("','", array(mediamosa_app_db::ACTIVE_FALSE, mediamosa_app_db::ACTIVE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::ACTIVE_FALSE, mediamosa_app_db::ACTIVE_TRUE)) . "')",
        'default' => mediamosa_app_db::ACTIVE_FALSE,
      ),
      mediamosa_app_db::ACTIVE_VERSION => array(
        'description' => 'Use as global versions for REST calls. When specified, use this version or lower of called REST calls.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::ACTIVE_VERSION_LENGTH,
        'not null' => TRUE,
        'default' => 0, // @TODO: which version???
      ),
      mediamosa_app_db::DESCRIPTION => array(
        'description' => 'The description of the client.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::DESCRIPTION_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::OWNER => array(
        'description' => 'De owner of the used drupal rol.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::OWNER_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::TECHNICAL_NAME => array(
        'description' => 'Technical name of the client contact person.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::TECHNICAL_NAME_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::TECHNICAL_PHONE => array(
        'description' => 'Technical phone of the client contact person.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::TECHNICAL_PHONE_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::TECHNICAL_EMAIL => array(
        'description' => 'Technical email address of the client contact person.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::TECHNICAL_EMAIL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ADMINISTRATIVE_NAME => array(
        'description' => 'Administrative name of the client contact person.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::ADMINISTRATIVE_NAME_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ADMINISTRATIVE_PHONE => array(
        'description' => 'Administrative phone of the client contact person.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::ADMINISTRATIVE_PHONE_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ADMINISTRATIVE_EMAIL => array(
        'description' => 'Administrative email of the client contact person.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::ADMINISTRATIVE_EMAIL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::PLAY_PROXY_URL => array(
        'description' => 'The URL to use for playing mediafiles.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::PLAY_PROXY_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::VIEW_ASSET_URL => array(
        'description' => 'The URL to view the mediafiles.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::VIEW_ASSET_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::PREVIEW_PROFILE_ID => array(
        'description' => 'Transcoding profile used for previewing.',
        'type' => 'int',
        'not null' => FALSE,
      ),
      mediamosa_app_db::DOWNLOAD_URL => array(
        'description' => 'Shown in the asset details rest calls. Available tags: asset_id, mediafile_id and mediafile_filename.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::DOWNLOAD_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::STREAM_URL => array(
        'description' => 'Shown in the asset details rest calls. Available tags: asset_id, mediafile_id and mediafile_filename.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::STREAM_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::STILL_URL => array(
        'description' => 'Shown by OAI for still images. Available tags: asset_id.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::STILL_URL_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_app_db::ALWAYS_HINT_MP4 => array(
        'description' => 'Always hint (make streamable) mp4 files.',
        'type' => "enum('" . implode("','", array(mediamosa_app_db::ALWAYS_HINT_MP4_FALSE, mediamosa_app_db::ALWAYS_HINT_MP4_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::ALWAYS_HINT_MP4_FALSE, mediamosa_app_db::ALWAYS_HINT_MP4_TRUE)) . "')",
        'default' => mediamosa_app_db::ALWAYS_HINT_MP4_TRUE,
      ),
      mediamosa_app_db::ALWAYS_INSERT_MD => array(
        'description' => 'Always insert extra metadata flv files.',
        'type' => "enum('" . implode("','", array(mediamosa_app_db::ALWAYS_INSERT_MD_FALSE, mediamosa_app_db::ALWAYS_INSERT_MD_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::ALWAYS_INSERT_MD_FALSE, mediamosa_app_db::ALWAYS_INSERT_MD_TRUE)) . "')",
        'default' => mediamosa_app_db::ALWAYS_INSERT_MD_TRUE,
      ),
      mediamosa_app_db::IS_METADATA_PUBLIC => array(
        'description' => 'Is metadata of this client application public visible in OAI?',
        'type' => "enum('" . implode("','", array(mediamosa_app_db::IS_METADATA_PUBLIC_FALSE, mediamosa_app_db::IS_METADATA_PUBLIC_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::IS_METADATA_PUBLIC_FALSE, mediamosa_app_db::IS_METADATA_PUBLIC_TRUE)) . "')",
        'default' => mediamosa_app_db::IS_METADATA_PUBLIC_TRUE,
      ),
      mediamosa_app_db::SHOW_EXTERNAL_ASSETS => array(
        'description' => 'Show external references in OAI (in field referenceId)?',
        'type' => "enum('" . implode("','", array(mediamosa_app_db::SHOW_EXTERNAL_ASSETS_FALSE, mediamosa_app_db::SHOW_EXTERNAL_ASSETS_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::SHOW_EXTERNAL_ASSETS_FALSE, mediamosa_app_db::SHOW_EXTERNAL_ASSETS_TRUE)) . "')",
        'default' => mediamosa_app_db::SHOW_EXTERNAL_ASSETS_TRUE,
      ),
      mediamosa_app_db::TRANSCODING_MAX_SLOTS => array(
        'description' => 'Set the maximum number of transcoding jobs allowed for this application.',
        'type' => 'int',
        'not null' => TRUE,
        'default' => 5,
      ),
      mediamosa_app_db::ALLOW_MASTERSLAVE_APPS => array(
        'description' => 'An array of application IDs as PHP serialized which are allowed to add master/slave.',
        'type' => 'text',
        'not null' => FALSE,
        'default' => NULL,
      ),
      mediamosa_app_db::OAUTH_CONSUMER_KEY => array(
        'description' => 'The identifier portion of the client credentials (username).',
        'type' => 'varbinary',
        'length' => mediamosa_app_db::OAUTH_CONSUMER_KEY_LENGTH,
        'not null' => FALSE,
        'default' => '',
      ),
      mediamosa_app_db::OAUTH_CONSUMER_SECRET => array(
        'description' => 'The password portion of the client credentials (password).',
        'type' => 'varbinary',
        'length' => mediamosa_app_db::OAUTH_CONSUMER_SECRET_LENGTH,
        'not null' => FALSE,
        'default' => '',
      ),
      mediamosa_app_db::TIMEZONE => array(
        'description' => 'The application timezone.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::TIMEZONE_LENGTH,
        'not null' => FALSE,
        'default' => mediamosa_settings::MEDIAMOSA_DEFAULT_TIMEZONE,
      ),
      mediamosa_app_db::STILL_DEFAULT_SIZE => array(
        'description' => 'The default size of the still image.',
        'type' => 'varchar',
        'length' => mediamosa_app_db::STILL_DEFAULT_SIZE_LENGTH,
        'not null' => FALSE,
        'default' => mediamosa_settings::STILL_DEFAULT_SIZE,
      ),
      mediamosa_app_db::STILL_PADDING => array(
        'description' => 'Force padding, if aspect ratio is maintaned.',
        'type' => "enum('" . implode("','", array(mediamosa_app_db::STILL_PADDING_NO, mediamosa_app_db::STILL_PADDING_YES)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::STILL_PADDING_NO, mediamosa_app_db::STILL_PADDING_YES)) . "')",
        'default' => mediamosa_app_db::STILL_PADDING_YES,
      ),
      mediamosa_app_db::STILL_MAINTAIN_ASPECT_RATIO => array(
        'description' => 'Force maintain the original video aspect ratio or not.',
        'type' => "enum('" . implode("','", array(mediamosa_app_db::STILL_MAINTAIN_ASPECT_RATIO_NO, mediamosa_app_db::STILL_MAINTAIN_ASPECT_RATIO_YES)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_app_db::STILL_MAINTAIN_ASPECT_RATIO_NO, mediamosa_app_db::STILL_MAINTAIN_ASPECT_RATIO_YES)) . "')",
        'default' => mediamosa_app_db::STILL_MAINTAIN_ASPECT_RATIO_YES,
      ),
      mediamosa_app_db::TOOL_ANALYSE => array(
        'description' => 'Serialized array to determine which tools will put analyse data to asset metadata.',
        'type' => 'text',
        'not null' => FALSE,
        'default' => NULL,
      ),
    ),
    'primary key' => array(mediamosa_app_db::NID), // Revisions are stored in mediamosa_node_revision.
    'unique keys' => array(
      'uni_appid' => array(mediamosa_app_db::APP_ID),
    ),
    'indexes' => array(
      'idx_' . mediamosa_app_db::OAUTH_CONSUMER_KEY => array(mediamosa_app_db::OAUTH_CONSUMER_KEY),
    ),
  );

  return $schema;
}
