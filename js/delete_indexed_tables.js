/**
(function ($){
    jQuery(document).ready(function(){
    	alert("Drupal is awesome!");
    }); // end alert

    Drupal.behaviors.myAlert = {
        attach: function(context, settings){
            $(document).ready(function(){
                alert('Thanks for using elastic search!');
            }); // end alert
        }
    };


})jQuery;
*/
