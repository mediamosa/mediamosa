<?php
/**
 * @file
 * The basic exeception class for MediaMosa Storage S3.
 */

/**
 * Basic exception class.
 */
class mediamosa_storage_s3_exception extends mediamosa_exception {

  const MEDIAMOSA_STORAGE_S3_EXCEPTION_SEVERITY_HIGH = 80;
  const MEDIAMOSA_STORAGE_S3_EXCEPTION_SEVERITY_NORMAL = 50;
  const MEDIAMOSA_STORAGE_S3_EXCEPTION_SEVERITY_LOW = 20;

  /**
   * Constructor.
   */
  public function __construct($message = '', $severity = self::MEDIAMOSA_STORAGE_S3_EXCEPTION_SEVERITY_NORMAL, $log_error = TRUE) {
    $message = empty($message) ? sprintf("MediaMosa Storage S3 exception was thrown.") : $message;

    parent::__construct(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, $message, $severity, $log_error);
  }
}
