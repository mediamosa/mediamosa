<?php
/**
 * @file
 * DB class job.
 */

class mediamosa_job_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_job';

  // Database fields;
  const ID = 'job_id';
  const APP_ID = 'app_id';
  const ASSET_ID = 'asset_id';
  const ASSET_ID_LENGTH = mediamosa_db::HASH_ID_LENGTH;
  const MEDIAFILE_ID = 'mediafile_id';
  const MEDIAFILE_ID_LENGTH = mediamosa_db::HASH_ID_LENGTH;
  const MEDIAFILE_DEST = 'mediafile_dest';
  const MEDIAFILE_DEST_LENGTH = mediamosa_db::HASH_ID_LENGTH;
  const FILENAME = 'filename';
  const FILENAME_LENGTH = 255;
  const OWNER_ID = 'owner';
  const OWNER_ID_LENGTH = 255;
  const JOB_STATUS = 'status';
  const JOB_STATUS_WAITING = 'WAITING';
  const JOB_STATUS_INPROGRESS = 'INPROGRESS';
  const JOB_STATUS_FINISHED = 'FINISHED';
  const JOB_STATUS_FAILED = 'FAILED';
  const JOB_STATUS_CANCELLED = 'CANCELLED';
  const JOB_STATUS_CANCELLING = 'CANCELING';
  const PROGRESS = 'progress';
  const PROGRESS_LENGTH = 5;
  const PRIORITY = 'priority';
  const JOB_TYPE = 'job_type';
  const JOB_TYPE_TRANSCODE = 'TRANSCODE';
  const JOB_TYPE_RETRANSCODE = 'RETRANSCODE';
  const JOB_TYPE_STILL = 'STILL';
  const JOB_TYPE_UPLOAD = 'UPLOAD';
  const JOB_TYPE_ANALYSE = 'ANALYSE';
  const JOB_TYPE_DELETE_MEDIAFILE = 'DELETE_MEDIAFILE';
  const JOB_TYPE_TRANSFER_MEDIA_UPLOAD = 'TRANSFER_MEDIA_UPLOAD';
  const JOB_TYPE_TRANSFER_MEDIA_DOWNLOAD = 'TRANSFER_MEDIA_DOWNLOAD';
  const JOB_TYPE_TRANSFER_MEDIA_MOVE = 'TRANSFER_MEDIA_MOVE';
  const STARTED = 'started';
  const FINISHED = 'finished';
  const ERROR_DESCRIPTION = 'error_description';
  const CREATED = 'created';
  const CHANGED = 'changed';
  const CREATE_STILL = 'create_still';
  const CREATE_STILL_TRUE = 'TRUE';
  const CREATE_STILL_FALSE = 'FALSE';
  const STILL_PARAMETERS = 'still_parameters';
  const HINT = 'hint';
  const HINT_TRUE = 'TRUE';
  const HINT_FALSE = 'FALSE';
  // TODO: Remove on the next release.
  const NO_HINT = 'no_hint';
  const NO_HINT_TRUE = 'TRUE';
  const NO_HINT_FALSE = 'FALSE';
}
