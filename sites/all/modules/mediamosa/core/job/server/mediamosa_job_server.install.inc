<?php
/**
 * @file
 * Install for mediamosa_job_server table.
 */

require_once 'mediamosa_job_server_db.class.inc';

require_once 'analyse/mediamosa_job_server_analyse.install.inc';
require_once 'still/mediamosa_job_server_still.install.inc';
require_once 'transcode/mediamosa_job_server_transcode.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_job_server_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_job_server_db::TABLE_NAME);

  // Create child tables;
  _mediamosa_job_server_analyse_install();
  _mediamosa_job_server_still_install();
  _mediamosa_job_server_transcode_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_job_server_uninstall() {
  // Drop child tables;
  _mediamosa_job_server_analyse_uninstall();
  _mediamosa_job_server_still_uninstall();
  _mediamosa_job_server_transcode_uninstall();

  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_job_server_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_job_server_schema() {

  $schema[mediamosa_job_server_db::TABLE_NAME] = array(
  'description' => 'The job server table.',
    'fields' => array(
      mediamosa_job_server_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_job_server_db::INSTALL_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_job_server_db::INSTALL_ID_LENGTH,
        'default' => 'default',
        'description' => 'The installion ID for multiple installments of jobserver.',
      ),
      mediamosa_job_server_db::JOB_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The job ID.',
      ),
      mediamosa_job_server_db::JOB_STATUS => array(
        'description' => 'The status of the job.',
        'type' => "enum('" . implode("','", array(mediamosa_job_server_db::JOB_STATUS_WAITING, mediamosa_job_server_db::JOB_STATUS_INPROGRESS, mediamosa_job_server_db::JOB_STATUS_FINISHED, mediamosa_job_server_db::JOB_STATUS_FAILED, mediamosa_job_server_db::JOB_STATUS_CANCELLED)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_server_db::JOB_STATUS_WAITING, mediamosa_job_server_db::JOB_STATUS_INPROGRESS, mediamosa_job_server_db::JOB_STATUS_FINISHED, mediamosa_job_server_db::JOB_STATUS_FAILED, mediamosa_job_server_db::JOB_STATUS_CANCELLED)) . "')",
        'default' => mediamosa_job_server_db::JOB_STATUS_WAITING,
      ),
      mediamosa_job_server_db::PROGRESS => array(
        'type' => 'varchar',
        'length' => mediamosa_job_server_db::PROGRESS_LENGTH,
        'not null' => TRUE,
        'description' => 'The progress of the job from 0.000 to 1.000.',
        'default' => '0.000',
      ),
      mediamosa_job_server_db::JOB_TYPE => array(
        'description' => 'The type of the job.',
        'type' => "enum('" . implode("','", array(mediamosa_job_server_db::JOB_TYPE_TRANSCODE, mediamosa_job_server_db::JOB_TYPE_STILL, mediamosa_job_server_db::JOB_TYPE_UPLOAD, mediamosa_job_server_db::JOB_TYPE_ANALYSE, mediamosa_job_server_db::JOB_TYPE_DELETE_MEDIAFILE, mediamosa_job_server_db::JOB_TYPE_TRANSFER_MEDIA_DOWNLOAD, mediamosa_job_server_db::JOB_TYPE_TRANSFER_MEDIA_UPLOAD, mediamosa_job_server_db::JOB_TYPE_TRANSFER_MEDIA_MOVE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_server_db::JOB_TYPE_TRANSCODE, mediamosa_job_server_db::JOB_TYPE_STILL, mediamosa_job_server_db::JOB_TYPE_UPLOAD, mediamosa_job_server_db::JOB_TYPE_ANALYSE, mediamosa_job_server_db::JOB_TYPE_DELETE_MEDIAFILE, mediamosa_job_server_db::JOB_TYPE_TRANSFER_MEDIA_DOWNLOAD, mediamosa_job_server_db::JOB_TYPE_TRANSFER_MEDIA_UPLOAD, mediamosa_job_server_db::JOB_TYPE_TRANSFER_MEDIA_MOVE)) . "')",
        'default' => mediamosa_job_server_db::JOB_TYPE_UPLOAD,
      ),
      mediamosa_job_server_db::PROGRESS_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'description' => 'Unused.',
        'default' => NULL,
      ),
      mediamosa_job_server_db::MEDIAFILE_ID_SRC => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'default' => NULL,
        'description' => 'The mediafile ID.',
      ),
      mediamosa_job_server_db::MEDIAFILE_DEST => array(
        'type' => 'text',
        'default' => NULL,
        'description' => 'The mediafile.',
      ),
      mediamosa_job_server_db::STARTED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when job was started.',
      ),
      mediamosa_job_server_db::FINISHED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when job was finished.',
      ),
      mediamosa_job_server_db::ERROR_DESCRIPTION => array(
        'type' => 'text',
        'size' => 'normal',
        'description' => 'Possible error description log.',
      ),
      mediamosa_job_server_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when job was created.',
      ),
      mediamosa_job_server_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when job was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_job_server_db::ID, mediamosa_job_server_db::INSTALL_ID),
    'indexes' => array(
      'idx_install_status' => array(mediamosa_job_server_db::INSTALL_ID, mediamosa_job_server_db::JOB_STATUS),
    ),
    'unique keys' => array(
      'uni_jobid' => array(mediamosa_job_db::TABLE_NAME => mediamosa_job_db::ID),
    ),
    'foreign keys' => array(
      mediamosa_job_server_db::JOB_ID => array(mediamosa_job_db::TABLE_NAME => mediamosa_job_db::ID),
    ),
  );

  return $schema;
}
