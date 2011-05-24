<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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

require_once 'mediamosa_statistics_diskspace_used_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_statistics_diskspace_used_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_statistics_diskspace_used_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_statistics_diskspace_used_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_statistics_diskspace_used_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_statistics_diskspace_used_schema() {

  $schema[mediamosa_statistics_diskspace_used_db::TABLE_NAME] = array(
  'description' => 'The statistics_diskspace_used hmmm table.',
    'fields' => array(
      mediamosa_statistics_diskspace_used_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_statistics_diskspace_used_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_statistics_diskspace_used_db::TYPE => array(
        'description' => 'The ticket type.',
        'type' => "enum('" . implode("','", array(mediamosa_statistics_diskspace_used_db::TYPE_CONTAINER, mediamosa_statistics_diskspace_used_db::TYPE_EUA, mediamosa_statistics_diskspace_used_db::TYPE_GROUP, mediamosa_statistics_diskspace_used_db::TYPE_USER)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_statistics_diskspace_used_db::TYPE_CONTAINER, mediamosa_statistics_diskspace_used_db::TYPE_EUA, mediamosa_statistics_diskspace_used_db::TYPE_GROUP, mediamosa_statistics_diskspace_used_db::TYPE_USER)) . "')",
        'description' => 'The type.',
      ),
      mediamosa_statistics_diskspace_used_db::KEYWORD => array(
        'type' => 'varchar',
        'length' => mediamosa_statistics_diskspace_used_db::KEYWORD_LENGTH,
        'not null' => TRUE,
        'description' => 'The keyword.',
      ),
      mediamosa_statistics_diskspace_used_db::CONTAINER_TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_statistics_diskspace_used_db::CONTAINER_TYPE_LENGTH,
        'not null' => TRUE,
        'description' => 'The keyword.',
      ),
      mediamosa_statistics_diskspace_used_db::DISKSPACE => array(
        'type' => 'int',
        'size' => 'normal',
        'not null' => TRUE,
        'description' => 'Diskspace.',
      ),
      mediamosa_statistics_diskspace_used_db::TIMESTAMP => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The time and date.',
      ),
    ),
    'primary key' => array(mediamosa_statistics_diskspace_used_db::ID),
  );

  return $schema;
}
