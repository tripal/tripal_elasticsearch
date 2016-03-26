(function ($){
	//jQuery(document).ready(function(){
	//	alert("Drupal is awesome!");
	//}); // end alert

	Drupal.behaviors.myAlert = {
		attach: function(context, settings){
			$(document).ready(function(){
				//alert('Thanks for using elastic search!');
				//$('div .elastic-search-form-item input').datepicker();
			}); // end alert
		}
	};

//	$(document).ready(function(){
//		$('div .elastic-search-form-item input').datepicker();
//	})

})(jQuery);
