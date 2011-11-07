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
 * Job Display functions.
 */
class mediamosa_job_display {

  /**
   * Get the job display data.
   * @param integer $job_id
   *  The job ID.
   */
  static public function get($job_id, $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_display_db::TABLE_NAME, 'js')
      ->fields('js', $fields)
      ->condition(mediamosa_job_display_db::ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Set hide attribute.
   *
   * @param $job_id
   */
  static public function hide($job_id) {
    if (!self::get($job_id)) {
      $query = mediamosa_db::db_insert(mediamosa_job_display_db::TABLE_NAME);
      $query->fields(array(
        mediamosa_job_display_db::ID => $job_id,
        mediamosa_job_display_db::VISIBILITY => mediamosa_job_display_db::VISIBILITY_HIDDEN,
      ));
      $query->execute();
    }
  }

  /**
   * Set hide attribute.
   *
   * @param $job_id
   */
  static public function unhide($job_id) {
    $num_deleted = mediamosa_db::db_delete(mediamosa_job_display_db::TABLE_NAME)
      ->condition(mediamosa_job_display_db::ID, $job_id)
      ->execute();
  }
}
