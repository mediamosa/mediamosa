<?php
/**
 * @file
 * Contains the global MediaMosa Rest calls.
 */

/**
 * URI: /version
 * Get the current version number.
 */
class mediamosa_rest_call_mediamosa_version extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Methods.
  /**
   * Implements get_var_setup().
   */
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  /**
   * Implements do_call().
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get version array.
    $version = mediamosa_version::get_current_version(TRUE);

    $mediamosa->add_item(
      array(
        'version' => mediamosa_version::get_current_version_str(FALSE),
        'build' => $version[mediamosa_version::BUILD],
        'info' => $version[mediamosa_version::INFO],
      )
    );
  }
}

/**
 * URI: /get_current_mount_point
 * Method: GET
 */
class mediamosa_rest_call_get_current_mount_point extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Methods.
  /**
   * Implements get_var_setup().
   */
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  /**
   * Implements do_call().
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the linux mount point.
    $wrapper = new mediamosa_io_streamwrapper_local();
    $current_mount_point = $wrapper->get_storage_path_linux();

    // Add to output.
    $mediamosa->add_item(array('current_mount_point' => $current_mount_point));
  }
}
