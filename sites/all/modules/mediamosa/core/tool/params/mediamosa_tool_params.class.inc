<?php
/**
 * @file
 * The tools parameter implementation.
 *
 * The tools parameters class handles the command parameters and mapping for
 * the tool.
 */

class mediamosa_tool_params {

  // ---------------------------------------------------------------- Functions.
  /**
   * Get tool_param listing.
   */
  public static function search() {
    return mediamosa_db::db_select(mediamosa_tool_params_db::TABLE_NAME, 'tm')
      ->fields('tm')
      ->execute();
  }

  /**
   * Get a specific tool parameter.
   *
   * @param string $tool
   *   The name of the tool.
   * @param string $nice_parameter
   *   The parameter name but more readable (nice).
   *
   * @return array
   *   The tool or FALSE.
   */
  public static function get_by_tool($tool, $nice_parameter = '') {

    $query = mediamosa_db::db_select(mediamosa_tool_params_db::TABLE_NAME, 'tp')->fields('tp');

    if ($nice_parameter) {
      return $query
        ->condition(mediamosa_tool_params_db::NICE_PARAMETER, $nice_parameter)
        ->condition(mediamosa_tool_params_db::TOOL, $tool)
        ->orderBy(mediamosa_tool_params_db::WEIGHT, 'ASC')
        ->orderBy(mediamosa_tool_params_db::NICE_PARAMETER)
        ->execute()
        ->fetchAssoc();
    }

    return $query->condition(mediamosa_tool_params_db::TOOL, $tool)
      ->orderBy(mediamosa_tool_params_db::WEIGHT, 'ASC')
      ->orderBy(mediamosa_tool_params_db::NICE_PARAMETER)
      ->execute();
  }

  /**
   * Check value against tools parameter settings.
   *
   * @param string $tool
   *   Tool used.
   * @param string $nice_parameter
   *   The param to check.
   * @param mixed $value
   *   The value to check.
   *
   * @throws mediamosa_exception_error().
   */
  public static function check_mapping($tool, $nice_parameter, $value) {

    // Get the mapping param.
    $transcode_mapping = self::get_by_tool($tool, $nice_parameter);

    if (!$transcode_mapping) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FOUND, array('@key' => $nice_parameter));
    }

    // Check if value is in range and right type.
    if (!is_null($transcode_mapping[mediamosa_tool_params_db::MIN_VALUE]) || !is_null($transcode_mapping[mediamosa_tool_params_db::MAX_VALUE])) {
      if (!is_numeric($value)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FLOAT, array('@key' => $nice_parameter, '@value' => $value));
      }

      if (!is_null($transcode_mapping[mediamosa_tool_params_db::MIN_VALUE])) {
        if ($transcode_mapping[mediamosa_tool_params_db::MIN_VALUE] > $value) {
          throw new mediamosa_exception_error(
            mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FLOAT,
            array(
              '@key' => $nice_parameter,
              '@min_value' => $transcode_mapping[mediamosa_tool_params_db::MIN_VALUE],
              '@value' => $value,
            )
          );
        }

        if ($transcode_mapping[mediamosa_tool_params_db::MAX_VALUE] < $value) {
          throw new mediamosa_exception_error(
            mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FLOAT,
            array(
              '@key' => $nice_parameter,
              '@max_value' => $transcode_mapping[mediamosa_tool_params_db::MAX_VALUE],
              '@value' => $value,
            )
          );
        }
      }
    }

    // If allowed values is set, then make sure its one of them.
    if (!empty($transcode_mapping[mediamosa_tool_params_db::ALLOWED_VALUE])) {
      $allowed_values = unserialize($transcode_mapping[mediamosa_tool_params_db::ALLOWED_VALUE]);
      if (!in_array($value, $allowed_values)) {
        return new mediamosa_exception_error(
          mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_WRONG_VALUE,
          array(
            '@key' => $nice_parameter,
            '@value' => $value,
          )
        );
      }
    }
  }

  /**
   * Get the tool param using its machine name.
   *
   * @param string $machine_name
   *   The machine name.
   *
   * @return array
   *   The MediaMosa tool param or FALSE.
   */
  public static function get_by_machinename($machine_name) {
    return mediamosa_db::db_select(mediamosa_tool_params_db::TABLE_NAME, 't')
      ->fields('t')
      ->condition(mediamosa_tool_params_db::MACHINE_NAME, $machine_name)
      ->execute()
      ->fetchAssoc();
  }
}
