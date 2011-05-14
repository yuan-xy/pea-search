	function info_or_error(ret,file, action){
						if(ret){
							show_info("执行成功! &nbsp;&nbsp;"+action+'"'+file+'".');
						}else{
							show_error("执行失败! &nbsp;&nbsp;"+action+'"'+file+'".');
						}
		}
	function hide_message(){
			$("#message-area").fadeTo(2500,0.0);
		}
        function show_error(msg){
			$("#message-area").addClass("error");
			$("#message-area").removeClass("info");
			$("#message-area").html(msg);
			$("#message-area").css("opacity", 1);
			setTimeout(hide_message,3000);
        }
        function show_info(msg){
			$("#message-area").addClass("info");
			$("#message-area").removeClass("error");
			$("#message-area").html(msg);
			$("#message-area").css("opacity", 1);
			setTimeout(hide_message,3000);
        }