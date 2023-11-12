var num_rows = 0;
var num_cols = 0;

var lightMode = true;
var info = false;
var create = false;
var error = false;

window.onload = function() 
{

};

function fillTable(data)
{
	//console.log(data);
	//document.getElementById("result").innerText = data;
	var data_arr = data.split("\n");

	var result_table = document.getElementById("result_table");

	if (num_rows == 0)
	{
		for (var i=0; i<data_arr.length; i++)
		{
			var row;
			var row_arr = data_arr[i].split(",");;

			if (i == 0)
				row = document.getElementById("col_names");
			else
			{
				row = document.createElement('tr');
				row.id = "row_"+i;
			}

			for (var j=0; j<row_arr.length; j++)
			{
				var cell;

				if (i == 0)
					cell = document.createElement('th');
				else
					cell = document.createElement('td');

				cell.innerHTML = row_arr[j];
				cell.id = "cell_"+i+"_"+j;
				row.append(cell);
			}

			if (i != 0)
				result_table.append(row);

			num_cols = row_arr.length;
		}

		num_rows = data_arr.length;
	}
	else
	{
		for (var i=0; i<data_arr.length; i++)
		{
			var row;
			var row_arr = data_arr[i].split(",");

			var new_row = false;

			if (i == 0)
				row = document.getElementById("col_names");
			else
				row = document.getElementById("row_"+i);

			if (i != 0 && row == null)
			{
				row = document.createElement('tr');
				row.id = "row_"+i;

				new_row = true;
			}

			for (var j=0; j<row_arr.length; j++)
			{
				var cell = document.getElementById("cell_"+i+"_"+j);

				if (cell == null)
				{
					if (i == 0)
						cell = document.createElement('th');
					else
						cell = document.createElement('td');

					cell.innerHTML = row_arr[j];
					cell.id = "cell_"+i+"_"+j;
					row.append(cell);
				}
				else
					cell.innerHTML = row_arr[j];
			}

			var k = row_arr.length;
			while(document.getElementById("cell_"+i+"_"+k) != null)
			{
			    document.getElementById("cell_"+i+"_"+k).remove();
			    k++;
			}

			if (new_row)
				result_table.append(row);

			num_cols = row_arr.length;
		}

		num_rows = data_arr.length;

		var k = data_arr.length;
		while(document.getElementById("row_"+k) != null)
		{
		    document.getElementById("row_"+k).remove();
		    k++;
		}
	}

	//document.getElementById("div_table").setAttribute("style","height:60%");
}

function sendCommand()
{
	var cmd = document.getElementById("query_area").value;
	cmd = cmd.replace("\n", ' ');

	let url = 'http://localhost:3000/cmd/?statement="'+cmd+'"';
	//console.log(url);
	fetch(url)
		.then(res => res.json())
		.then(data => 
		{
			fillTable(data);
		})
	.catch(err => showHideError());
}

function getTables()
{
	let url = 'http://localhost:3000/get_tables/';
	console.log(url);
	fetch(url)
		.then(res => res.json())
		.then(data => 
		{
			var tables_list = document.getElementById("tables_list");

			while (tables_list.firstChild) 
			{
		        tables_list.removeChild(tables_list.firstChild);
		    }

			var tables_arr = data.split(";");
			for (var i=0; i<tables_arr.length; i++)
			{
				var table = document.createElement("li");
				
				var name = document.createElement("span");
				name.className = "caret";
				name.innerHTML = tables_arr[i].split(":")[0];

				table.append(name);

				var nested = document.createElement("ul");
				nested.className = "nested";

				var cols_arr = tables_arr[i].split(":")[1].split(",");

				for (var j=0; j<cols_arr.length; j++)
				{
					var col = document.createElement("li");
					col.innerHTML = cols_arr[j];
					//col.style.display = "none";

					nested.append(col);
				}

				table.append(nested);

				tables_list.append(table);
			}

			var toggler = document.getElementsByClassName("caret");

			for (var i=0; i<toggler.length; i++) 
			{
			  	//console.log(toggler[i].innerHTML);
			  	toggler[i].addEventListener("click", function() 
			  	{
			    	this.parentElement.querySelector(".nested").classList.toggle("active");
			    	this.classList.toggle("caret-down");
			  	});
			}
		})
	.catch(err => showHideError());
}

function createTable()
{
	var num_rows = document.getElementById("num_rows").value;
	var table_name = document.getElementById("table_name").value;

	if (table_name == "")
		alert("Please enter a valid table name");
	else if (num_rows == "" || isNaN(num_rows))
		alert("Please enter a valid number of rows");
	else
	{
		let url = 'http://localhost:3000/create_csv/?num_rows='+num_rows+'&table_name='+table_name;
		console.log(url);
		fetch(url)
			.then(res => res.json())
			.then(data => 
			{
				fillTable(data);
			})
		.catch(err => showHideError());
	}
}

function switchMode()
{
	if (lightMode)
	{
		document.body.style.background = "darkgreen";
		document.getElementById("mode").innerHTML = "Light Mode";

		var elements = document.getElementsByClassName("content");
	    for (var i = 0; i < elements.length; i++) 
	    {
	        elements[i].style.backgroundColor = "#262626";
	        elements[i].style.color = "white";
	    }

	    var elements = document.getElementsByClassName("modal-content");
	    for (var i = 0; i < elements.length; i++) 
	    {
	        elements[i].style.backgroundColor = "#262626";
	        elements[i].style.color = "white";
	    }

	    document.getElementById("col_names").style.backgroundColor = "darkgreen";

		lightMode = false;
	}
	else
	{
		document.body.style.background = "lightgreen";
		document.getElementById("mode").innerHTML = "Dark Mode";

		var elements = document.getElementsByClassName("content");
	    for (var i = 0; i < elements.length; i++) 
	    {
	        elements[i].style.backgroundColor = "white";
	        elements[i].style.color = "black";
	    }

	    var elements = document.getElementsByClassName("modal-content");
	    for (var i = 0; i < elements.length; i++) 
	    {
	        elements[i].style.backgroundColor = "white";
	        elements[i].style.color = "black";
	    }

	    document.getElementById("col_names").style.backgroundColor = "lightgreen";

		lightMode = true;
	}
}

function showHideInfo()
{
	if (!info)
	{
		document.getElementById("info_modal").style.display = "block";
		info = true;
	}
	else
	{
		document.getElementById("info_modal").style.display = "none";
		info = false;
	}
}

function showHideCreate()
{
	if (!create)
	{
		document.getElementById("create_modal").style.display = "block";
		create = true;
	}
	else
	{
		document.getElementById("create_modal").style.display = "none";
		create = false;
	}
}

function showHideError()
{
	if (!error)
	{
		document.getElementById("error_modal").style.display = "block";
		error = true;
	}
	else
	{
		document.getElementById("error_modal").style.display = "none";
		error = false;
	}
}