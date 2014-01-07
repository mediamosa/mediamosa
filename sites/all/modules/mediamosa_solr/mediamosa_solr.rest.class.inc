<?php
/**
 * @file
 * REST calls specific for use inside the simpletest sandbox.
 */

/**
 * URI: /cron/solr
 *
 * This url is triggered by the cron.
 *
 * Internal only.
 *
 */
class mediamosa_rest_call_solr_process_queue extends mediamosa_rest_call {

  public function get_var_setup() {
    $a_var_setup = array(
      mediamosa_rest_call::VARS => array(),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Reindex what we can.
    mediamosa_solr::processQueue();

    // Add info to output.
    $mediamosa->set_result_okay();
  }
}
