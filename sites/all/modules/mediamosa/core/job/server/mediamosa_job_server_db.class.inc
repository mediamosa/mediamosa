<?php
/**
 * @file
 * DB class jobserver.
 */

class mediamosa_job_server_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_job_server';

  // Database fields;
  const ID = 'jobserver_job_id';
  const INSTALL_ID = 'install_id';
  const INSTALL_ID_LENGTH = 16;
  const JOB_ID = 'job_id';
  const JOB_STATUS = 'status';
  const JOB_STATUS_WAITING = 'WAITING';
  const JOB_STATUS_INPROGRESS = 'INPROGRESS';
  const JOB_STATUS_FINISHED = 'FINISHED';
  const JOB_STATUS_FAILED = 'FAILED';
  const JOB_STATUS_CANCELLED = 'CANCELLED';
  const PROGRESS = 'progress';
  const PROGRESS_LENGTH = 5;
  const JOB_TYPE = 'job_type';
  const JOB_TYPE_TRANSCODE = 'TRANSCODE';
  const JOB_TYPE_STILL = 'STILL';
  const JOB_TYPE_UPLOAD = 'UPLOAD';
  const JOB_TYPE_ANALYSE = 'ANALYSE';
  const JOB_TYPE_DELETE_MEDIAFILE = 'DELETE_MEDIAFILE';
  const JOB_TYPE_TRANSFER_MEDIA_UPLOAD = 'TRANSFER_MEDIA_UPLOAD';
  const JOB_TYPE_TRANSFER_MEDIA_DOWNLOAD = 'TRANSFER_MEDIA_DOWNLOAD';
  const JOB_TYPE_TRANSFER_MEDIA_MOVE = 'TRANSFER_MEDIA_MOVE';
  const PROGRESS_ID = 'progress_id';
  const MEDIAFILE_ID_SRC = 'mediafile_src';
  const MEDIAFILE_ID_SRC_LENGTH = 32;
  const MEDIAFILE_DEST = 'mediafile_dest';
  const STARTED = 'started';
  const FINISHED = 'finished';
  const ERROR_DESCRIPTION = 'error_description';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
