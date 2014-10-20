<?php
/**
 * @file
 * The Asset Metadata Property REST calls.
 */

/**
 * URI: /metadata_tag
 * Get a listing of the current asset media metadata define
 *
 * 1.x: media_management_metadata_definition_get
 */
class mediamosa_rest_call_asset_metadata_property_list extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // REST vars.
  const INCLUDE_DEFAULT_DEFINITIONS = 'include_default_definitions'; // Switch if we need to get all the default definitions that are 'global' over all apps.
  const NAME = 'name';  // Search on specific name.

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array(
      self::VARS => array(
        self::INCLUDE_DEFAULT_DEFINITIONS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DEFAULT_VALUE => 'TRUE',
          self::DESCRIPTION => "Switch if we need to get all the default definitions that are 'global' over all apps.",
        ),
        self::NAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM_UNDERSCORE,
          self::DESCRIPTION => 'Optional property name to search on.',
        ),
      ),
    );

    // Enrich with default REST vars limit / offset.
    $a_var_setup = self::get_var_setup_range($a_var_setup);

    // Add order by stuff.
    $a_var_setup = self::get_var_setup_order_by(
      $a_var_setup,
      NULL, // No default.
      array(
        mediamosa_asset_metadata_property_db::ID,
        mediamosa_asset_metadata_property_db::NAME,
        mediamosa_asset_metadata_property_db::TYPE,
        mediamosa_asset_metadata_property_db::CREATED,
        mediamosa_asset_metadata_property_db::CHANGED,
      ),
      mediamosa_type::ORDER_DIRECTION_ASC
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();

    // limit, offset.
    $limit = $this->get_param_value(self::LIMIT);
    $offset = $this->get_param_value(self::OFFSET);

    // Order by, order direction.
    $order_by = $this->get_param_value(self::ORDER_BY);
    $order_direction = $this->get_param_value(self::ORDER_DIRECTION);

    // Include all default properties?
    $include_default_definitions = $this->get_param_value(self::INCLUDE_DEFAULT_DEFINITIONS);

    // Name to search on.
    $name = $this->get_param_value(self::NAME);

    // Get all properties.
    $properties = mediamosa_asset_metadata_property::property_list(
      $app_ids,
      $include_default_definitions,
      $name,
      $offset,
      $limit,
      $order_by,
      $order_direction
    );

    foreach ($properties as $property) {
      $mediamosa->add_item(
        array(
          'prop_id' => $property[mediamosa_asset_metadata_property_db::ID],
          'name' => $property[mediamosa_asset_metadata_property_db::NAME],
          'prop_group' => $property[mediamosa_asset_metadata_property_group_db::NAME],
          'type' => $property[mediamosa_asset_metadata_property_db::TYPE],
          'created' => mediamosa::utcdate2appdate($property[mediamosa_asset_metadata_property_db::CREATED]),
          'changed' => mediamosa::utcdate2appdate($property[mediamosa_asset_metadata_property_db::CHANGED]),
          'is_hidden' => $property[mediamosa_asset_metadata_property_db::IS_HIDDEN],
          'is_protected' => $property[mediamosa_asset_metadata_property_db::IS_PROTECTED],
        )
      );
    }
  }
}


/**
 * URI: /metadata_tag/create
 * Create a asset metadata property.
 */
class mediamosa_rest_call_asset_metadata_property_create extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const NAME = 'name';
  const TYPE = 'type';
  const IS_HIDDEN = 'is_hidden';
  const IS_PROTECTED = 'is_protected';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array(
      mediamosa_rest_call::VARS => array(
        self::NAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM_UNDERSCORE_TAG,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::DESCRIPTION => 'The name of the property to create.',
          self::VAR_RANGE_END => mediamosa_asset_metadata_property_db::NAME_LENGTH,
        ),
        self::TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_ALLOWED_VALUES => array('datetime', 'int', 'char'),
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::DESCRIPTION => "The type of data to be stored, can either be 'datetime' (iso-8601 standard date), 'int' (numbers), char (text).",
        ),
        self::IS_HIDDEN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_ALLOWED_VALUES => array(mediamosa_asset_metadata_property_db::IS_HIDDEN_FALSE, mediamosa_asset_metadata_property_db::IS_HIDDEN_TRUE),
          self::VAR_DEFAULT_VALUE => mediamosa_asset_metadata_property_db::IS_HIDDEN_FALSE,
          self::DESCRIPTION => 'True if the property is hidden.',
        ),
        self::IS_PROTECTED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_ALLOWED_VALUES => array(mediamosa_asset_metadata_property_db::IS_PROTECTED_FALSE, mediamosa_asset_metadata_property_db::IS_PROTECTED_TRUE),
          self::VAR_DEFAULT_VALUE => mediamosa_asset_metadata_property_db::IS_PROTECTED_FALSE,
          self::DESCRIPTION => 'True if the property is protected against removal when asset metadata replace is used. Protected properties can not be unset or lose their value.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Name of the new property.
    $name = $this->get_param_value(self::NAME);

    // Type of the new property.
    $type = $this->get_param_value(self::TYPE);

    // Is the new property hidden.
    $is_hidden = $this->get_param_value(self::IS_HIDDEN);

    $is_protected = $this->get_param_value(self::IS_PROTECTED);

    // Create the property. Any problems will throw exception.
    $property_id = mediamosa_asset_metadata_property::property_create($app_id, $name, $type, 0, 0, $is_hidden, $is_protected);

    // Get the property.
    $property = mediamosa_asset_metadata_property::property_get_row($property_id);

    assert($property);

    // If we get here, it was successful.
    $mediamosa->add_item($property);
  }
}

/**
 * URI: /metadata_tag/delete
 *
 * Delete a asset metadata property.
 */
class mediamosa_rest_call_asset_metadata_property_delete extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const NAME = 'name';
  const DELETE = 'delete';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array(
      mediamosa_rest_call::VARS => array(
        self::NAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM_UNDERSCORE,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::DESCRIPTION => 'The name of the property to delete.',
          self::VAR_RANGE_END => mediamosa_asset_metadata_property_db::NAME_LENGTH,
        ),
        self::DELETE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_ALLOWED_VALUES => array('cascade', ''),
          self::DESCRIPTION => "Specify as 'cascade' to delete all metadata as well. Deletion will fail if you try to delete an property that still has metadata.",
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Name of the new property.
    $name = $this->get_param_value(self::NAME);

    // Type of the new property.
    $delete = $this->get_param_value(self::DELETE);

    // Create the property. Any problems will throw exception.
    mediamosa_asset_metadata_property::property_delete($app_id, $name, $delete);

    // If we get here, it was successful. We have no output, but we are ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /tagcount
 *
 * Count the tags per tag per prop_id.
 *
 * FIXME: Why this weird rest call? ITS DB HEAVY. Maybe do group by on
 * val_char_lft.
 */
class mediamosa_rest_call_asset_metadata_property_tagcount extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const PROP_ID = 'prop_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array(
      mediamosa_rest_call::VARS => array(
        self::PROP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::DESCRIPTION => 'The prop ID of which the count must be calculated..',
        ),
      ),
    );

    // Include the limit, offset.
    $a_var_setup = self::get_var_setup_range($a_var_setup);

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Range.
    $limit = $this->get_param_value_limit();
    $offset = $this->get_param_value_offset();

    // Prop ID.
    $prop_id = $this->get_param_value(self::PROP_ID);

    // Get the count.
    $result = mediamosa_asset_metadata::tagcount($app_id, $prop_id, $offset, $limit);

    // Set the result.
    $item = array();
    foreach ($result as $row) {
      $item['count'] = $row['tagcount'];
      $item['tag'] = $row[mediamosa_asset_metadata_db::VAL_CHAR];
      $mediamosa->add_item($item);
    }
  }
}
