<?php
/**
 * @file
 * API documentation MediaMosa Solr.
 */

/**
 * @addtogroup hooks
 * @{
 */


/**
 * Change the version shown.
 *
 * @param array $version
 *   The version array;
 *   - 'major': The major version.
 *   - 'minor': The minor part of version.
 *   - 'release': The release part of version.
 *   - 'build': The current build.
 *   - 'info': The info string. This field is optional and can be used to
 *     indicate special platform or version.
 */
/**
 * Allow adding or coping metadata to the Solr document object.
 *
 * @param Apache_Solr_Document $document
 *   The Solr document that is indexed.
 * @param array $names
 *   - 'group_name' like 'dublin_core'
 *   - 'group_name_short' like 'dc'.
 *   - 'name' like 'title'.
 * @param array $metadata_index
 *   The name of the metadata field, e.g. 'dc_title'.
 * @param array $asset_for_index
 *   The asset metadata data collect using the asset.
 */
function hook_mediamosa_solr_document_metadata_alter($document, $names, $asset_for_index) {
}

/**
 * Alter the acl query part for Solr search.
 *
 * @param array $query
 *   The query array.
 * @param array $parameters
 *   The solr parameters.
 */
function hook_mediamosa_solr_acl_search_alter(&$query, $parameters = array()) {
}

/**
 * Alter the acl query part for Solr search.
 *
 * @param array $query
 *   The query array.
 * @param array $parameters
 *   The solr parameters.
 */
function hook_mediamosa_solr_search_alter(&$query, $parameters = array()) {
}

/**
 * Alter the metadata array when indexing document.
 *
 * @param array $asset_for_index
 *   The asset metadata data collect using the asset.
 */
function hook_mediamosa_solr_asset_alter(&$asset_for_index) {
}

/**
 * Alter the asset document when indexing document.
 *
 * @param Apache_Solr_Document $document
 *   The Solr document that is indexed.
 * @param array $asset_for_index
 *   The asset metadata data collect using the asset.
 */
function hook_mediamosa_solr_document_asset_alter($document, &$asset_for_index) {
}

/**
 * Alter the Solr query parts.
 *
 * @param array $parts
 *   The query parts to change.
 * @param array $parameters
 *   The solr parameters.
 */
function hook_mediamosa_solr_cql_parts_alter(&$parts, $parameters = array()) {

}

/**
 * Allows altering the facet result data before it returned to mediamosa.
 *
 * @param mediamosa_solr_apache_solr_response $mediamosa_solr_apache_solr_response
 *   The mediamosa solr apache response.
 * @param Apache_Solr_HttpTransport_Response $apache_solr_http_transport_response
 *   The mediamosa solr apache response.
 * @param array $facets
 *   Array with facet result data.
 */
function hook_mediamosa_solr_apache_facet_alter($mediamosa_solr_apache_solr_response, $apache_solr_http_transport_response, $facets) {

}

/**
 * Allows altering the facet result data before it returned to mediamosa.
 *
 * @param mediamosa_solr_apache_solr_response $mediamosa_solr_apache_solr_response
 *   The mediamosa solr apache response.
 * @param Apache_Solr_HttpTransport_Response $apache_solr_http_transport_response
 *   The mediamosa solr apache response.
 * @param array $related
 *   Array with related result data.
 */
function hook_mediamosa_solr_apache_related_alter($mediamosa_solr_apache_solr_response, $apache_solr_http_transport_response, $related) {

}

/**
 * @} End of "addtogroup hooks".
 */
