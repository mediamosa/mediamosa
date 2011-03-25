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
  * The MediaMosa node revision table stores revision data of MediaMosa content
  * types. Because we only use revisions for history, the difference between
  * revisions is stored as an serialized array.
  */

require_once 'mediamosa_node_revision_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_node_revision_install() {

  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_node_revision_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_node_revision_uninstall() {
  // Remove our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_node_revision_db::TABLE_NAME);
}
/**
 * Implements hook_schema().
 */
function mediamosa_node_revision_schema() {

  $schema[mediamosa_node_revision_db::TABLE_NAME] = array(
    'description' => 'The history data of our nodes is stored here.',
    'fields' => array(
      mediamosa_node_revision_db::NID => array(
        'description' => 'The Drupal node id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_node_revision_db::VID => array(
        'description' => 'The Drupal node revision id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_node_revision_db::REVISION_DATA => array(
        'description' => 'The data of the revision, stored as an serialized array.',
        'type' => 'text',
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_app_db::NID, mediamosa_app_db::VID),
  );

  return $schema;
}
