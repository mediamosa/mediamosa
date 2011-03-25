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
  * The FTP Batch module
  */
class mediamosa_ftp_batch {

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Get the list of batches.
   *
   * @param integer $app_id
   *  The application ID.
   * @param integer $offset
   *  Offset in the result list.
   * @param unknown_type $limit
   *  Limit of items in the result list.
   */
  static public function search($app_id, $offset, $limit) {

    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'batch_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'owner_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'group_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'vuf';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'started';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'finished';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'email_address';
    $a_query[mediamosa_db_query::A_FROM][] = strtr('{#mediamosa_ftp_batch}', array('#mediamosa_ftp_batch' => mediamosa_ftp_batch_db::TABLE_NAME));

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = 'app_id = ' . $app_id;
    $a_query[mediamosa_db_query::A_ORDER_BY][] = "batch_id ASC";

    $a_query[mediamosa_db_query::LIMIT] = $limit;

    // Build the query.
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));

    // Do the query.
    $result = mediamosa_db::db_query($query);

    // Set total found rows.
    $result->found_rows = mediamosa_db::db_found_rows();

    return $result;
  }

  /**
   * Get a batch.
   *
   * @param integer $batch_id
   *  The batch ID.
   *
   * @return array
   *  The found batch or FALSE.
   */
  static public function get($batch_id) {

    // Get the batch.
    return mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_ftp_batch} WHERE #batch_id = :batch_id',
      array(
        '#mediamosa_ftp_batch' => mediamosa_ftp_batch_db::TABLE_NAME,
        '#batch_id' => mediamosa_ftp_batch_db::ID,
        ':batch_id' => $batch_id
      )
    )->fetchAssoc();
  }

  /**
   * Create the ftp_batch.
   *
   * @param integer $app_id
   *  The application ID.
   * @param string $owner_id
   *  The owner ID.
   * @param string $group_id
   *  The group ID.
   *
   * @return integer
   *  Returns the created batch ID.
   */
  static public function create($app_id, $owner_id, $group_id) {

    $a_fields = array(
      mediamosa_ftp_batch_db::APP_ID => $app_id,
      mediamosa_ftp_batch_db::OWNER_ID => $owner_id,
      mediamosa_ftp_batch_db::GROUP_ID => $group_id,
      mediamosa_ftp_batch_db::VUF => '',
    );

    // No created/changed fields.

    return mediamosa_db::db_insert(mediamosa_ftp_batch_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Update the ftp batch
   *
   * @param $app_id
   *  The application ID. (unused, future)
   * @param $batch_id
   *  The batch ID.
   * @param $owner_id
   *  The owner ID.
   * @param $group_id
   *  The group ID.
   * @param $vuf
   *  VUF value.
   * @param $started
   *  Started time batch.
   * @param $finished
   *  Finished time batch.
   * @param $email_address
   *  Email address.
   * @param $email_contents
   */
  static public function update($app_id, $batch_id, $owner_id, $group_id, $vuf, $started, $finished, $email_address, $email_contents) {

    $a_fields = array(
      mediamosa_ftp_batch_db::OWNER_ID => $owner_id,
      mediamosa_ftp_batch_db::GROUP_ID => $group_id,
      mediamosa_ftp_batch_db::VUF => $vuf,
      mediamosa_ftp_batch_db::STARTED => $started,
      mediamosa_ftp_batch_db::FINISHED => $finished,
      mediamosa_ftp_batch_db::EMAIL_ADDRESS => $email_address,
      mediamosa_ftp_batch_db::EMAIL_CONTENTS => $email_contents,
    );

    // No changed/created fields.

    mediamosa_db::db_update(mediamosa_ftp_batch_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_ftp_batch_db::ID, $batch_id)
      ->execute();
  }

  /**
   * Delete a ftp batch.
   *
   * @param integer $batch_id
   *  ID of batch to delete.
   */
  static public function delete($batch_id) {

    // Remove relations between this batch and his assets.
    mediamosa_ftp_batch_asset::remove_asset_relation($batch_id);

    // Delete the batch.
    mediamosa_db::db_query("DELETE FROM {#mediamosa_ftp_batch} WHERE #batch_id = :batch_id",
      array(
        '#mediamosa_ftp_batch' => mediamosa_ftp_batch_db::TABLE_NAME,
        '#batch_id' => mediamosa_ftp_batch_db::ID,
        ':batch_id' => $batch_id
      )
    );
  }
}
