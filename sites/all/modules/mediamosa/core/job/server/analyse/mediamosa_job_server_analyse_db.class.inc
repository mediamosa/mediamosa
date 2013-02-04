<?php
/**
 * @file
 * Server job analyse table.
 */

class mediamosa_job_server_analyse_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_job_server_analyse'; // 1.x name jobserver_analyse_job.

  // Database fields;
  const ID = 'jobserver_job_id';
  const ANALYSE_RESULT = 'analyse_result';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
