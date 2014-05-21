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
 * Job Upload functions.
 */
class mediamosa_job_upload {

  /**
   * Get job upload.
   *
   * @param int $job_id
   */
  public static function get($job_id) {
    return mediamosa_db::db_select(mediamosa_job_upload_db::TABLE_NAME, 'ju')
      ->fields('ju')
      ->condition(mediamosa_job_upload_db::JOB_ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  public static function in_progress($mediafile_id, $job_id) {
    $a_statuses = array(
      mediamosa_job_db::JOB_STATUS_WAITING,
      mediamosa_job_db::JOB_STATUS_INPROGRESS,
      mediamosa_job_db::JOB_STATUS_CANCELLING
    );

    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'mj')
      ->fields('mj', array(mediamosa_job_db::ID))
      ->condition(mediamosa_job_db::JOB_TYPE, mediamosa_job_db::JOB_TYPE_UPLOAD)
      ->condition(mediamosa_job_db::MEDIAFILE_ID, $mediafile_id)
      ->condition(mediamosa_job_db::ID, $job_id, '<>')
      ->condition(mediamosa_job_db::JOB_STATUS, $a_statuses, 'IN')
      ->countQuery()->execute()->fetchField() > 0 ? TRUE : FALSE;
  }

  /**
   * Create a upload job.
   *
   * @param int $app_id
   *  The application ID.
   * @param int $job_id
   *  The job ID.
   * @param string $user_id
   *  Owner/user.
   * @param string $group_id
   *  Group ID.
   * @param string $mediafile_id
   *  Mediafile ID.
   * @param int $file_size
   *  Filesize.
   * @param bool $retranscode
   * @param bool $create_still
   */
  public static function create($app_id, $job_id, $user_id, $group_id, $mediafile_id, $file_size, $retranscode = FALSE, $create_still = FALSE) {

    // If already upload job is here, then dont add it.
    if (self::in_progress($mediafile_id, $job_id)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UPLOAD_ALREADY_EXISTS);
    }

    // Data to insert.
    $a_fields = array(
      mediamosa_job_upload_db::JOB_ID => $job_id,
      mediamosa_job_upload_db::FILE_SIZE => $file_size,
      mediamosa_job_upload_db::RETRANSCODE => $retranscode ? mediamosa_job_upload_db::RETRANSCODE_TRUE : mediamosa_job_upload_db::RETRANSCODE_FALSE,
      mediamosa_job_upload_db::CREATE_STILL => $create_still ? mediamosa_job_upload_db::CREATE_STILL_TRUE : mediamosa_job_upload_db::CREATE_STILL_FALSE,
    );

    // Add created value.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    $job_id_upload = mediamosa_db::db_insert(mediamosa_job_upload_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();

    // Log the upload event.
    mediamosa_statistics::log_event_file_upload($app_id, $user_id, $group_id, $file_size);
  }
}

