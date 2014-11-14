<?php
/**
 * @file
 * The REST calls for the error module.
 */

/**
 * URI: /errorcodes/
 * Retrieve a listing of the current Error in the database.
 */
class mediamosa_rest_call_error_list extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    // Enrich with limit and offset vars.
    $a_var_setup = $this->get_var_setup_range();

    // Enrich with Order by and Order direction vars.
    $a_var_setup = $this->get_var_setup_order_by(
      $a_var_setup,
      mediamosa_error_db::CODE,
      array(
        mediamosa_error_db::CODE,
        mediamosa_error_db::NAME,
      )
    );

    // My vars for this REST call.
    $a_var_setup[mediamosa_rest_call::VARS][mediamosa_error_db::CODE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
      mediamosa_rest_call::VAR_RANGE_START => mediamosa_error_db::CODE_RANGE_START,
      mediamosa_rest_call::VAR_RANGE_END => mediamosa_error_db::CODE_RANGE_END,
      mediamosa_rest_call::VAR_DESCRIPTION => mediamosa::t('The ID of the error code to retrieve.'),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the variables.
    $code = $this->get_param_value(mediamosa_error_db::CODE);
    $order_by = $this->get_param_value(self::ORDER_BY);
    $order_direction = $this->get_param_value(self::ORDER_DIRECTION);
    $limit = $this->get_param_value(self::LIMIT);
    $offset = $this->get_param_value(self::OFFSET);

    if ($code) {
      $code = mediamosa_error::error_code_find($code, array(), FALSE);
      if (!$code) {
        return;
      }

      $codes = array($code);
    }
    else {
      $codes = mediamosa_error::error_code_list($order_by, $order_direction);
    }

    foreach ($codes as $code) {
      if ($offset && $offset--) {
        continue;
      }

      $o_mediamosa->add_item(array(
        mediamosa_error_db::NAME => $code[mediamosa_error_db::NAME],
        mediamosa_error_db::CODE => $code[mediamosa_error_db::CODE],
        mediamosa_error_db::MESSAGE => $code[mediamosa_error_db::MESSAGE],
      ));

      if (!--$limit) {
        break;
      }
    }
  }
}

/**
 * URI: /errorcodes/#$code
 * Retrieve a specific error from the current database.
 */
class mediamosa_rest_call_error_get extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    // My vars for this REST call.
    $a_var_setup[mediamosa_rest_call::VARS][mediamosa_error_db::CODE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
      mediamosa_rest_call::VAR_RANGE_START => mediamosa_error_db::CODE_RANGE_START,
      mediamosa_rest_call::VAR_RANGE_END => mediamosa_error_db::CODE_RANGE_END,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the dbus line.
    $code = $this->get_param_value(mediamosa_error_db::CODE);

    // Get the error code.
    $a_code = mediamosa_error::error_code_find($code, array(), FALSE);

    if ($a_code) {
      $o_mediamosa->add_item($a_code);
    }
  }
}
