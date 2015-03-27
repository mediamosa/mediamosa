<?php
/**
 * @file
 * Tool params installer.
 */

require_once 'mediamosa_tool_params_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_tool_params_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_tool_params_db::TABLE_NAME);

  // Create our node type.
  $tool_params_node_type = array(
    'type' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_PARAMS,
    'name' => st('MediaMosa tool params'),
    'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_PARAMS,
    'description' => st('The tool params contains information about the tool (eg. ffmpeg, windows) parameters.'),
    'custom' => 1,
    'modified' => 1,
    'locked' => 0,
    'has_body' => 0,
  );

  $tool_params_node_type = node_type_set_defaults($tool_params_node_type);
  node_type_save($tool_params_node_type);
}

/**
 * Implements hook_unstall().
 */
function _mediamosa_tool_params_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_tool_params_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_tool_params_schema() {

  $schema[mediamosa_tool_params_db::TABLE_NAME] = array(
  'description' => 'The tool parameter table.',
    'fields' => array(
      mediamosa_tool_params_db::NID => array(
        'description' => 'The Drupal node id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_tool_params_db::VID => array(
        'description' => 'The Drupal node revision id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_tool_params_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_tool_params_db::MACHINE_NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_params_db::MACHINE_NAME_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => 'The MediaMosa tool params machine name.',
      ),
      mediamosa_tool_params_db::TOOL => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_params_db::TOOL_LENGTH,
        'description' => 'Tool.',
      ),
      mediamosa_tool_params_db::WEIGHT => array(
        'type' => 'int',
        'description' => 'The weight of the tool parameter for usage in order when generating the command line parameters.',
        'default' => 0,
      ),
      mediamosa_tool_params_db::NICE_PARAMETER => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_params_db::NICE_PARAMETER_LENGTH,
        'description' => 'Nice parameter (name of the parameter).',
      ),
      mediamosa_tool_params_db::TOOL_PARAMETER => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_params_db::TOOL_PARAMETER_LENGTH,
        'description' => 'Tool parameter.',
      ),
      mediamosa_tool_params_db::MIN_VALUE => array(
        'type' => 'int',
        'description' => 'The minimum value.',
        'default' => NULL,
      ),
      mediamosa_tool_params_db::MAX_VALUE => array(
        'type' => 'int',
        'description' => 'The maximum value.',
        'default' => NULL,
      ),
      mediamosa_tool_params_db::ALLOWED_VALUE => array(
        'type' => 'text',
        'description' => 'Allowed value.',
      ),
      mediamosa_tool_params_db::DEFAULT_VALUE => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_params_db::DEFAULT_VALUE_LENGTH,
        'description' => 'Default value.',
      ),
      mediamosa_tool_params_db::REQUIRED => array(
        'description' => 'It is a required parameter or not.',
        'type' => "enum('" . implode("','", array(mediamosa_tool_params_db::REQUIRED_FALSE, mediamosa_tool_params_db::REQUIRED_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_tool_params_db::REQUIRED_FALSE, mediamosa_tool_params_db::REQUIRED_TRUE)) . "')",
        'default' => mediamosa_tool_params_db::REQUIRED_TRUE,
      ),
      mediamosa_tool_params_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when tool parameter was created.',
      ),
      mediamosa_tool_params_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when tool parameter was changed last.',
      ),
      mediamosa_tool_params_db::DESCRIPTION => array(
        'type' => 'text',
        'description' => 'Description of the parameter.',
      ),
      mediamosa_tool_params_db::TYPE_PARAMETER => array(
        'description' => 'The type of the parameter.',
        'type' => "enum('" . implode("','", array(mediamosa_tool_params_db::TYPE_PARAMETER_SELECT, mediamosa_tool_params_db::TYPE_PARAMETER_CHECKBOX)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_tool_params_db::TYPE_PARAMETER_SELECT, mediamosa_tool_params_db::TYPE_PARAMETER_CHECKBOX)) . "')",
        'default' => mediamosa_tool_params_db::TYPE_PARAMETER_SELECT,
      ),
    ),
    'primary key' => array(mediamosa_tool_params_db::ID),
    'unique keys' => array(
      'uni_niceparam_tool' => array(mediamosa_tool_params_db::NICE_PARAMETER, mediamosa_tool_params_db::TOOL),
      'uni_machine_name' => array(mediamosa_server_db::MACHINE_NAME),
    ),
    'indexes' => array(
      'idx_tool' => array(mediamosa_tool_params_db::TOOL),
    )
  );

  return $schema;
}
