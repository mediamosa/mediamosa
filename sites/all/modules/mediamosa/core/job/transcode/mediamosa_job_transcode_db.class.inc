<?php
/**
 * @file
 * Transcode job table.
 */
class mediamosa_job_transcode_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_job_transcode'; // 1.x name transcode_job.

  // Database fields;
  const ID = 'transcode_job_id';
  const JOB_ID = 'job_id';
  const TRANSCODE_PROFILE_ID = 'transcode_profile_id';
  const TOOL = 'tool';
  const TOOL_LENGTH = 50;
  const COMMAND = 'command';
  const FILE_EXTENSION = 'file_extension';
  const FILE_EXTENSION_LENGTH = 5;
  const CREATED = 'created';
  const CHANGED = 'changed';
  const COMPLETED_TRANSCODING_URL = 'completed_transcoding_url';
}
