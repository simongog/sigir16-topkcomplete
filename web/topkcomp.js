/*jslint  browser: true, white: true, plusplus: true */
/*global $, countries */

$(function () {
    'use strict';

    // Initialize ajax autocomplete:
    $('#autocomplete-dynamic').autocomplete({
        noCache: true,
        serviceUrl: '/topcomp',
        dataType: 'json',
        lookupFilter: function(suggestion, originalQuery, queryLowerCase) {
            var re = new RegExp('\\b' + $.Autocomplete.utils.escapeRegExChars(queryLowerCase), 'gi');
            return re.test(suggestion.value);
        },
        onSelect: function(suggestion) {
            //window.location = 'http://google.com/maps?q=' + suggestion.value;
            window.open('http://google.com/maps?q=' + suggestion.value);
//            alert("You selected:" + suggestion.value);
//            $('#selction-ajax').html('You selected: ' + suggestion.value + ', ' + suggestion.data);
        },
        onHint: function (hint) {
            $('#autocomplete-ajax-x').val(hint);
        },
        onInvalidateSelection: function() {
            $('#selction-ajax').html('You selected: none');
        }
    });
});
