(function ($){

  jQuery(document).ready(function(){

    $(".hit_description_all").hide();

    $("#block-tripal_elasticsearch-elastic_search_tables td").hover(
      function(){
        $(this).css("background-color", "#6699ff");
        $(this).find(".hit_description_teaser").hide();
        $(this).find(".hit_description_all").show();
      }, // end hover
      function(){
        $(this).css("background-color", "#F7F7F7");
        $(this).find(".hit_description_all").hide();
        $(this).find(".hit_description_teaser").show();
      }
    ); // end hover
  }); // end alert

})(jQuery)
