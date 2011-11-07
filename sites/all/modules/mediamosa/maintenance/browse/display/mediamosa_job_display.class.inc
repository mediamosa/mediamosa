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
      ->condition(mediamosa_job_display_db::JOB_ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Set hide attribute.
   *
   */
  static public function hide($job_id) {

    if(self::get($job_id))
    {
       // Create the insert object.
      $query = mediamosa_db::db_update(mediamosa_job_display_db::TABLE_NAME);

      // Insert values.
      $fields = array(
        mediamosa_job_display_db::JOB_ID => $job_id,
        mediamosa_job_display_db::VISIBILITY => mediamosa_job_display_db::VISIBILITY_HIDDEN,
      );

      // Enrich with created/changed.
      $fields = mediamosa_db::db_update_enrich($fields);

      return $query->fields($fields)->condition(mediamosa_job_display_db::JOB_ID, $job_id)->execute();
    }
    else
    {
      // Create the insert object.
      $query = mediamosa_db::db_insert(mediamosa_job_display_db::TABLE_NAME);

      // Insert values.
      $fields = array(
        mediamosa_job_display_db::JOB_ID => $job_id,
        mediamosa_job_display_db::VISIBILITY => mediamosa_job_display_db::VISIBILITY_HIDDEN,
      );

      // Enrich with created/changed.
      $fields = mediamosa_db::db_insert_enrich($fields);
       return $query->fields($fields)->execute();
    }

    return;
  }

  /**
   * Set hide attribute.
   *
   */
  static public function unhide($job_id) {

    if(self::get($job_id))
    {
       // Create the insert object.
      $query = mediamosa_db::db_update(mediamosa_job_display_db::TABLE_NAME);

      // Insert values.
      $fields = array(
        mediamosa_job_display_db::JOB_ID => $job_id,
        mediamosa_job_display_db::VISIBILITY => mediamosa_job_display_db::VISIBILITY_DISPLAY,
      );

      // Enrich with created/changed.
      $fields = mediamosa_db::db_update_enrich($fields);

      // Insert/update it.
      return $query->fields($fields)->condition(mediamosa_job_display_db::JOB_ID, $job_id)->execute();
    }
    else
    {
      // Create the insert object.
      $query = mediamosa_db::db_insert(mediamosa_job_display_db::TABLE_NAME);

      // Insert values.
      $fields = array(
        mediamosa_job_display_db::JOB_ID => $job_id,
        mediamosa_job_display_db::VISIBILITY => mediamosa_job_display_db::VISIBILITY_DISPLAY,
      );

      // Enrich with created/changed.
      $fields = mediamosa_db::db_insert_enrich($fields);

      return $query->fields($fields)->execute();
    }

    return;
   
    
  }
}