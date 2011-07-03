function get_file_path(tr){
	if(tr.tagName=="LI"){
		path = tr.title; 
		file = $(tr).find("span")[0].innerHTML;
	}else if(tr.tagName=="UL"){
		path = tr.firstChild.title; 
		file = $(tr).find("span")[0].innerHTML;
	}else{
		var inputs = $(tr).find("td input");
		type = inputs[0].value;
		file = $(tr).find("td[colid=0] span")[0].title;
		if(file=="") file = inputs[1].value;
		path = $(tr).find("td[colid=1] span")[0].title;
		if(path=="") path = inputs[2].value;
	}
}

function dblclick_file(target){
	get_file_path(target.parentNode);
	var ret = document.getElementById('plugin0').shell2_default(path+file);
	info_if_error(ret, file, "打开");
}

function dblclick_path(target){
	get_file_path(target.parentNode);
	ret = document.getElementById('plugin0').shell_explore(path);
	info_if_error(ret, path, "资源管理器");
}

var context_menu_obj = {
				  menuStyle: {
					width : "150px"
				  },
				  itemStyle: {
				  },
				  itemHoverStyle: {
				  },
/* 
				  onShowMenu: function(e, menu) {
					if ($(e.target).parents().find("input")[0].value != 'dir') { //性能有问题
					  $('#paste', menu).remove();
					}
					return menu;
				  },
*/
				  bindings: {
					  'default': function(t,menuitem) {
					    get_file_path(t);
						var ret = document.getElementById('plugin0').shell2_default(path+file);
						info_if_error(ret, file, menuitem.lastChild.data);
					  },
					  'openas': function(t,menuitem) {
					    get_file_path(t);
						var ret = document.getElementById('plugin0').shell2_openas(path+file);
						info_if_error(ret, file, menuitem.lastChild.data);
					  },
					  'explore': function(t,menuitem) {
					    get_file_path(t);
						if(type=="dir") ret = document.getElementById('plugin0').shell_explore(path+file);
						else  ret = document.getElementById('plugin0').shell_explore(path);
						info_if_error(ret, file, menuitem.lastChild.data);
					  },
					  'copypath': function(t,menuitem) {
					    get_file_path(t);
						var ret = document.getElementById('plugin0').copy_str(path+file);
						info_or_error(ret, path+file, menuitem.lastChild.data);
					  },
					  'copy': function(t,menuitem) {
					    get_file_path(t);
						var ret = document.getElementById('plugin0').shell2(path+file, "copy");
						info_or_error(ret, file, menuitem.lastChild.data);
					  },
					  'cut': function(t,menuitem) {
					    get_file_path(t);
						var ret = document.getElementById('plugin0').shell2(path+file, "cut");
						info_or_error(ret, file, menuitem.lastChild.data);
					  },
					  'paste': function(t,menuitem) {
					    get_file_path(t);
						if(type=="dir"){
							ret = document.getElementById('plugin0').shell2(path+file, "paste");
							info_or_error(ret, file, menuitem.lastChild.data);
						}else{
							show_error("粘贴的目标必须是文件夹，不能是文件.");
						}
					  },
					  'delete': function(t,menuitem) {
					    get_file_path(t);
						var ret = document.getElementById('plugin0').shell2(path+file, "delete");
						info_or_error(ret, file, menuitem.lastChild.data);
					  },
					  'prop': function(t,menuitem) {
					    get_file_path(t);
						var ret = document.getElementById('plugin0').shell2_prop(path+file);
						info_if_error(ret, file, menuitem.lastChild.data);
					  }
					}
				  }