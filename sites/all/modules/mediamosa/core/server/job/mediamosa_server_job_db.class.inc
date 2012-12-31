<?php
/**
 * @file
 * DB class jobserver.
 */

class mediamosa_server_job_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_server_job';

  // Database fields;
  const SERVER_ID = 'server_id';
  const JOB_ID = 'job_id';
  const PROGRESS = 'progress';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
