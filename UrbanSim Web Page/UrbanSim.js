require([
	"dojo/dom",
	"dojo/dom-construct",
	"dojo/query",
	"dojo/dom-class",
	"dojo/on",
	"dojo/_base/lang"
], function(dom, domConstruct, query, domClass, on, lang){
	/*MAIN*/
	var i,j;

	var employmentParams = query(".employment-input-parameters")[0];
	var householdParams = query(".household-input-parameters")[0];
	var tableSelector = query(".table-select")[0];
	var tableCreatorButton = query(".create-table-button")[0];
	var advancedParamsButton = query(".advanced-parameters-button")[0];
	var simpleParamsButton = query(".simple-parameters-button")[0];
	var useAdvancedParams = query(".use-advanced-parameters")[0];

	on(tableSelector, "change", swapTable);
	on(tableCreatorButton, "click", function(){
		downloadCSV(tableSelector.value);
	});
	on(advancedParamsButton, "click", showAdvancedParams);
	on(simpleParamsButton, "click", showSimpleParams);

	swapTable();
	buildAdvancedParams("employment");
	buildAdvancedParams("household");

	/*FUNCTIONS*/
	function showSimpleParams(){
		domClass.add(simpleParamsButton, "hidden");
		domClass.remove(advancedParamsButton, "hidden");
		let innerParams = query("." + tableSelector.value + "-input-parameters .parameters")[0];
		let advancedParams = query("." + tableSelector.value + "-input-parameters .advanced-parameters")[0];
		let advancedTitle = query("." + tableSelector.value + "-input-parameters .advanced-parameter-title")[0];
		let title = query("." + tableSelector.value + "-input-parameters .parameter-title")[0];
		domClass.remove(innerParams, "hidden");
		domClass.add(advancedParams, "hidden");
		domClass.remove(title, "hidden");
		domClass.add(advancedTitle, "hidden");
	}

	function buildAdvancedParams(tableType){
		let advancedParams = query("." + tableType + "-input-parameters .advanced-parameters")[0];
		let	startDate = query("." + tableType + "-input-parameters .parameters .param[name=\"startDate\"]")[0];
		let	stopDate = query("." + tableType + "-input-parameters .parameters .param[name=\"stopDate\"]")[0];
		let	advancedParamsTemplate = query("." + tableType + "-input-parameters .advanced-parameters-template")[0];

		domConstruct.empty(advancedParams);
		for(i=parseInt(startDate.value)+5; i<parseInt(stopDate.value)+1; i=i+5){
			var placed = lang.clone(advancedParamsTemplate);
			domConstruct.place(placed, advancedParams, "last");
			placed.querySelector(".accordion").innerHTML = i;
			on(placed.querySelector(".accordion"), "click", function(){
				this.classList.toggle("active");
				var panel = this.nextElementSibling;
				if (panel.style.maxHeight){
					panel.style.maxHeight = null;
				}else{
					panel.style.maxHeight = panel.scrollHeight + "px";
				}
			});
			domClass.remove(placed, "hidden");
		}
	}

	function showAdvancedParams(){
		domClass.add(advancedParamsButton, "hidden");
		domClass.remove(simpleParamsButton, "hidden");
		let	innerParams = query("." + tableSelector.value + "-input-parameters .parameters")[0];
		let	advancedParams = query("." + tableSelector.value + "-input-parameters .advanced-parameters")[0];
		let	advancedTitle = query("." + tableSelector.value + "-input-parameters .advanced-parameter-title")[0];
		let	title = query("." + tableSelector.value + "-input-parameters .parameter-title")[0];
		domClass.add(title, "hidden");
		domClass.remove(advancedTitle, "hidden");
		domClass.add(innerParams, "hidden");
		domClass.remove(advancedParams, "hidden");
		buildAdvancedParams(tableSelector.value);
	}

	function swapTable(){
		if(tableSelector.value == "employment"){
			domClass.remove(employmentParams, "hidden");
			domClass.add(householdParams, "hidden");
		}else if(tableSelector.value == "household"){
			domClass.remove(householdParams, "hidden");
			domClass.add(employmentParams, "hidden");
		}
		showSimpleParams();
	}

	function downloadCSV(dlName){
		var params = {};
		var rows, csvs;
		if(dlName == "household"){
			let rows2, rows3;

			let list = householdParams.getElementsByClassName("parameters")[0].getElementsByClassName("param");
			for(i=0; i<list.length; i++){
				params[list[i].name] = list[i].value;
			}
			let advParams = buildAdvParam(params, dlName);

			rows = [["year", "total_number_of_households"]];
			rows2 = [["year", "total_number_of_households", "income_min", "income_max"]];
			rows3 = [["year", "total_number_of_households", "tenure"]];

			let numHouseholds = parseInt(advParams[advParams.startDate].numHouseholds);
			for(i=advParams.startDate; i<advParams.stopDate+1; i++){
				let appender = 0;
				if(useAdvancedParams.checked){ appender = parseInt((i-advParams.startDate)/5)*5; }
				let year = advParams.startDate + appender;
				if(i != advParams.startDate){
					numHouseholds = numHouseholds*parseFloat(advParams[year].annualGrowthRate);
				}

				rows.push([i, numHouseholds]);

				rows2.push([i, numHouseholds*(parseFloat(advParams[year].percentUnder)/100.0), "min", "max"]);
				rows2.push([i, numHouseholds*(parseFloat(advParams[year].percentOver)/100.0), "min", "max"]);

				rows3.push([i, numHouseholds*(parseFloat(advParams[year].tenureOwn)/100.0), 1]);
				rows3.push([i, numHouseholds*(parseFloat(advParams[year].tenureRent)/100.0), 2]);
			}

			csvs = [[rows, dlName+"_total"], [rows2, dlName+"_income"], [rows3, dlName+"_tenure"]];
		}else if(dlName == "employment"){
			let list = employmentParams.getElementsByClassName("parameters")[0].getElementsByClassName("param");
			for(i=0; i<list.length; i++){
				params[list[i].name] = list[i].value;
			}
			var sectorList = employmentParams.getElementsByClassName("parameters")[0].getElementsByClassName("employment-sector");
			for(i=0; i<sectorList.length; i++){
				params[sectorList[i].name] = sectorList[i].value;
			}
			let advParams = buildAdvParam(params, dlName);

			rows = [["year", "total_number_of_jobs", "sector_id"]];

			let countyTotal = parseInt(advParams[advParams.startDate].countyTotal);
			for(i=advParams.startDate; i<advParams.stopDate+1; i++){
				let appender = 0;
				if(useAdvancedParams.checked){ appender = parseInt((i-advParams.startDate)/5)*5; }
				let year = advParams.startDate + appender;
				if(i != advParams.startDate){
					countyTotal = countyTotal*parseFloat(advParams[year].annualGrowthRate);
				}
				for(j=0; j<sectorList.length; j++){
					let jobCount = (parseFloat(advParams[year][sectorList[j].name])/100.0)*countyTotal;
					rows.push([i, jobCount, sectorList[j].name]);
				}
			}

			csvs = [[rows, dlName+"_total"]];
		}

		csvs.forEach(function(csvRows){
			let csvContent = "data:text/csv;charset=utf-8,";
			csvRows[0].forEach(function(rowArray){
				let row = rowArray.join(",");
				csvContent += row + "\r\n";
			});

			let encodedUri = encodeURI(csvContent);
			let link = document.createElement("a");
			link.setAttribute("href", encodedUri);
			link.setAttribute("download", csvRows[1] + ".csv");
			document.body.appendChild(link); // Required for FF
			link.click();
		});
	}

	function buildAdvParam(params, tableType){
		var advParams = {};
		var startDate = parseInt(params.startDate);
		var stopDate = parseInt(params.stopDate);
		advParams[startDate] = params;
		advParams["startDate"] = startDate;
		advParams["stopDate"] = stopDate;
		delete params.startDate;
		delete params.stopDate;
		var advList = query("." + tableType + "-input-parameters .advanced-parameters .advanced-parameters-template");
		for(i=0; i<advList.length; i++){
			var year = advList[i].children[0].innerHTML;
			var data = advList[i].children[1].children;
			advParams[year] = {};
			for(j=0; j<data.length; j++){
				if(data[j].name){ advParams[year][data[j].name] = data[j].value; }
			}
		}
		return advParams;
	}
});
