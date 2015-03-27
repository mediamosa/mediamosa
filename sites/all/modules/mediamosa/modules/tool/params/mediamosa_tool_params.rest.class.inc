<?php
/**
 * @file
 * Transcode parameter REST calls.
 */

/**
 * URI: /transcode/parameter
 * Method: GET
 */
class mediamosa_rest_call_tool_params_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.

  // ---------------------------------------------------------------- Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    foreach (mediamosa_tool_params::search() as $transcode_mapping) {
      // TODO: MediaMosa supports now a ";" separated list, but it is possible, that some allowed values has ";".
      $transcode_mapping[mediamosa_tool_params_db::ALLOWED_VALUE] = implode(';', (array) unserialize($transcode_mapping[mediamosa_tool_params_db::ALLOWED_VALUE]));
      $transcode_mapping[mediamosa_tool_params_db::CREATED] = mediamosa::utcdate2appdate($transcode_mapping[mediamosa_tool_params_db::CREATED]);
      $transcode_mapping[mediamosa_tool_params_db::CHANGED] = mediamosa::utcdate2appdate($transcode_mapping[mediamosa_tool_params_db::CHANGED]);

      $o_mediamosa->add_item($transcode_mapping);
    }
  }
}
