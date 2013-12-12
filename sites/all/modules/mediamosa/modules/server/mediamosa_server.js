(function ($) {

/**
 * Move a block in the blocks table from one region to another via select list.
 *
 * This behavior is dependent on the tableDrag behavior, since it uses the
 * objects initialized in that behavior to update the row.
 */
Drupal.behaviors.viewerDrag = {
  attach: function (context, settings) {
    // tableDrag is required and we should be on the blocks admin page.
    if (typeof Drupal.tableDrag === 'undefined' || typeof Drupal.tableDrag.viewers === 'undefined') {
      return;
    }

    var table = $('table#viewers');
    var tableDrag = Drupal.tableDrag.viewers; // Get the blocks tableDrag object.

    // Add a handler so when a row is dropped, update fields dropped into new regions.
    tableDrag.onDrop = function () {
      dragObject = this;
      var selectField = $('select.viewer-status-select', dragObject.rowObject.element);
      if ($(dragObject.rowObject.element).prev('tr').is('.viewer-status-enabled')) {
        selectField.val('ENABLED');
      }
      else if ($(dragObject.rowObject.element).prev('tr').is('.viewer-status-disabled')) {
        selectField.val('DISABLED');
      }
    };
  }
};

})(jQuery);
