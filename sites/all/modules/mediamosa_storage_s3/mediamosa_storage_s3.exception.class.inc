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

  public function __construct($msg = '', $severity = self::MEDIAMOSA_STORAGE_S3_EXCEPTION_SEVERITY_NORMAL, $log_error = TRUE) {
    $msg = empty($msg) ? sprintf("MediaMosa Storage S3 exception was thrown.") : $msg;

    // Low is not logged by us.
    if ($severity >= self::MEDIAMOSA_STORAGE_S3_EXCEPTION_SEVERITY_NORMAL) {
      $log_serverity = $severity >= self::MEDIAMOSA_STORAGE_S3_EXCEPTION_SEVERITY_HIGH ? WATCHDOG_CRITICAL : WATCHDOG_NOTICE;

      // Get the trace, if any.
      $backtrace = $this->getTraceAsString();

      if ($log_error) {
        mediamosa_watchdog::log(
          '@message (Exception thrown on line @line, in @file)<br />Partial backtrace: <pre>@backtrace</pre>',
          array(
            '@message' => $msg,
            '@line' => $this->getLine(),
            '@file' => $this->getFile(),
            '@backtrace' => empty($backtrace) ? 'not available' : $backtrace,
          ),
          $log_serverity,
          'MediaMosa Storage S3'
        );
      }
    }

    parent::__construct($msg);
  }
}
