function get_file_path(tr){
	//alert(tr.innerHTML);
	if(tr.tagName=="DIV"){
		path = $(tr).find("input")[1].value;
		file = $(tr).find("input")[0].value;
	}else if(tr.tagName=="UL"){
		alert(tr.tagName);
	}else{
		tds = $(tr).find("td");
		file = tds[0].title;
		path = tds[1].title;
		type = tds[2].value;
	}
}

function delete_file(t){
	get_file_path(t);
	if(window.confirm("您确定要删除 '"+file+"' 吗?")){
		var ret = cef.plugin.shell2(path+file, "delete");
		refresh();
		info_or_error(ret, file, "删除");
	}
}
function copy_file(t){
	get_file_path(t);
	var ret = cef.plugin.shell2(path+file, "copy");
	info_or_error(ret, file, "复制");
}
function cut_file(t){
	get_file_path(t);
	var ret = cef.plugin.shell2(path+file, "cut");
	info_or_error(ret, file, "剪切");
}

function dblclick_file(target){
	get_file_path(target);
	var ret = cef.plugin.shell2_default(path+file);
	info_if_error(ret, file, "打开");
}


function dblclick_path(target){
	get_file_path(target);
	ret = cef.plugin.shell_explore(path);
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
                  onContextMenu : function(event, menu){
                                  var rowId = $(event.target).parent("tr").attr("id");
								  console.log(rowId);
                                  var grid = $("#maintable");
                                  grid.setSelection(rowId);                   
                                  return true;
                  },
				  bindings: {
					  'default': function(t,menuitem) {
					    get_file_path(t);
						var ret = cef.plugin.shell2_default(path+file);
						info_if_error(ret, file, menuitem.lastChild.data);
					  },
					  'openas': function(t,menuitem) {
					    get_file_path(t);
						var ret = cef.plugin.shell2_openas(path+file);
						info_if_error(ret, file, menuitem.lastChild.data);
					  },
					  'explore': function(t,menuitem) {
					    get_file_path(t);
						if(type=="dir") ret = cef.plugin.shell_explore(path+file);
						else  ret = cef.plugin.shell_explore(path);
						info_if_error(ret, file, menuitem.lastChild.data);
					  },
					  'copypath': function(t,menuitem) {
					    get_file_path(t);
						var ret = cef.plugin.copy_str(path+file);
						info_or_error(ret, path+file, menuitem.lastChild.data);
					  },
					  'copy': function(t,menuitem) {
						copy_file(t);
					  },
					  'cut': function(t,menuitem) {
						cut_file(t);
					  },
					  'paste': function(t,menuitem) {
					    get_file_path(t);
						if(type=="dir"){
							ret = cef.plugin.shell2(path+file, "paste");
							info_or_error(ret, file, menuitem.lastChild.data);
						}else{
							show_error("粘贴的目标必须是文件夹，不能是文件.");
						}
					  },
					  'delete': function(t,menuitem) {
						delete_file(t);
					  },
					  'prop': function(t,menuitem) {
					    get_file_path(t);
						var ret = cef.plugin.shell2_prop(path+file);
						info_if_error(ret, file, menuitem.lastChild.data);
					  }
					}
				  }

var context_menu_obj3 = {
				  menuStyle: {
					width : "150px"
				  },
				  itemStyle: {
				  },
				  itemHoverStyle: {
				  },
				  bindings: {
					  'default': function(t,menuitem) {
						var fs = gird_selectd_files();
						var ret = cef.plugin.batch_open(fs);
						info_or_error(ret, fs, menuitem.lastChild.data);
					  },
					  'copy': function(t,menuitem) {
						var fs = gird_selectd_files();
						var ret = cef.plugin.batch_copy(fs);
						info_or_error(ret, fs, menuitem.lastChild.data);
					  },
					  'cut': function(t,menuitem) {
						var fs = gird_selectd_files();
						var ret = cef.plugin.batch_cut(fs);
						info_or_error(ret, fs, menuitem.lastChild.data);
					  },
					  'delete': function(t,menuitem) {
						var fs = gird_selectd_files();
						if(window.confirm("您确定要删除 '"+file+"' 吗?")){
							var ret = cef.plugin.batch_delete(fs);
							refresh();
							info_or_error(ret, fs, menuitem.lastChild.data);
						}
					  }
					}
				  }