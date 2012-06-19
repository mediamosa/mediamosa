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
 * The install file for asset media still style.
 */

require_once 'mediamosa_asset_mediafile_still_style.class.inc';
require_once 'mediamosa_asset_mediafile_still_style_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_mediafile_still_style_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_mediafile_still_style_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_mediafile_still_style_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_mediafile_still_style_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_mediafile_still_style_schema() {

  $schema[mediamosa_asset_mediafile_still_style_db::TABLE_NAME] = array(
  'description' => 'The asset mediafile still style table.',
    'fields' => array(
      mediamosa_asset_mediafile_still_style_db::ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_asset_mediafile_still_style_db::ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The name of image style.',
      ),
      mediamosa_asset_mediafile_still_style_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the asset.',
      ),
    ),
    'primary key' => array(mediamosa_asset_mediafile_still_style_db::ID),
    'indexes' => array(
      'idx_' . mediamosa_asset_mediafile_still_style_db::APP_ID => array(mediamosa_asset_mediafile_still_style_db::APP_ID),
    ),
    //'foreign keys' => array(
    //  mediamosa_asset_mediafile_still_style_db::ID => array('image_styles' => 'name'),
    //  mediamosa_asset_mediafile_still_style_db::APP_ID => array(mediamosa_app_db::TABLE_NAME => mediamosa_app_db::APP_ID),
    //),
  );

  return $schema;
}
