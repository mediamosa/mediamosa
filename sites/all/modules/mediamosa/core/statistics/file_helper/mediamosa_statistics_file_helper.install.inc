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
 * Install for statistics used diskspace.
 */

require_once 'mediamosa_statistics_file_helper_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_statistics_file_helper_install() {

  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_statistics_file_helper_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_statistics_file_helper_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_statistics_file_helper_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_statistics_file_helper_schema() {

  $schema[mediamosa_statistics_file_helper_db::TABLE_NAME] = array(
  'description' => 'The statistics file helper table.',
    'fields' => array(
      mediamosa_statistics_file_helper_db::FILE => array(
        'type' => 'varbinary',
        'length' => mediamosa_statistics_file_helper_db::FILE_LENGTH,
        'not null' => TRUE,
        'description' => 'The name of file.',
      ),
    ),
    'primary key' => array(mediamosa_statistics_file_helper_db::FILE),
  );

  return $schema;
}
