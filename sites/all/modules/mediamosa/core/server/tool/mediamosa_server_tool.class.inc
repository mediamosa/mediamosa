<?php
/**
 * @file
 * The server tool class.
 */

class mediamosa_server_tool {
  // ---------------------------------------------------------------- Functions.
  /**
   * Return the job processor server tools.
   *
   * @param boolean $get_full_tools_only
   *   Supply TRUE to get tools that have other hooks. Is temporary solution
   *   till tool & job re-write of 3.6.
   *
   * @return array
   *  An associative array with the info about the tools.
   */
  public static function get_server_tools($get_full_tools_only = TRUE) {
    // Return the registered tools by hook.
    $tools = module_invoke_all('mediamosa_tool_info');

    // Add default flags.
    foreach (array_keys($tools) as $tool_id) {
      $tools[$tool_id] += array(
        // Temporary solution till 3.6.
        'is_full_tool' => TRUE,
      );

      // Only full tools.
      if ($get_full_tools_only && !$tools[$tool_id]['is_full_tool']) {
        unset($tools[$tool_id]);
      }
    }

    return $tools;
  }
}
