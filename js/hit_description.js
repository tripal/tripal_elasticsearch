(function ($){

  jQuery(document).ready(function(){
    //$.msg(content: "the page is ready");
    $(".hit_description_all").hide();

    $("td").hover(
      function(){
        $(this).css("background-color", "#6699ff");
        $(this).find(".hit_description_teaser").hide();
        $(this).find(".hit_description_all").show();
      }, // end click
      function(){
        $(this).css("background-color", "#F7F7F7");
        $(this).find(".hit_description_all").hide();
        $(this).find(".hit_description_teaser").show();
      }
    ); // end click
  }); // end alert

})(jQuery)
