<?php
/**
 * @file
 * Default Exception class.
 */

/**
 * The basic mediamosa_solr_exception class
 */
class mediamosa_solr_exception extends mediamosa_exception {

  const MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_HIGH = 80;
  const MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_NORMAL = 50;
  const MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_LOW = 20;

  public function __construct($msg = '', $severity = self::MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_NORMAL, $log_error = TRUE) {
    $msg = empty($msg) ? sprintf("MediaMosa Solr exception was thrown.") : $msg;

    // Low is not logged by us.
    if ($severity >= self::MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_NORMAL) {
      $log_serverity = $severity >= self::MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_HIGH ? WATCHDOG_CRITICAL : WATCHDOG_NOTICE;

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
          'MediaMosa Solr'
        );
      }
    }

    parent::__construct($msg);
  }
}
