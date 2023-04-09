# Datasets
This directory containts the description of collections of datasets publically available on the internet. At the moment reading of text (ASCII) files only is supported.

To add a new collection of datasets, two steps are required.

----
## Register new collection:
To register a new collection, add the required information to the file *DatasetCollections.json* which has the following simple structure:

```javascript
[
	{
		"name" : "unique name of the collection",
		"description" : "description of the colleciton, can contain basic html-tags like <b>, etc. to format the text.",
		"url" : "URL of the collection to document where the data is taken from"
	},
	...
]
```

----
## Document datasets:
The documentation of the actual data sets for every collection is done in a separate file *collection_name.json*. This file should have the structure as in a following example for the dataset 'babyboom' from the JSE collection (*JSEDataArchive.json*):

```javascript
{
    "name": "JSEDataArchive",
    "categories": [
        {
            "name": "Category",
            "subcategories": [
                {
                    "name": "Sub-Category1",
                    "datasets": [
                        {
                            "description": "Time of Birth, Sex, and Birth Weight of 44 Babies",
                            "description_url": "http://jse.amstat.org/datasets/babyboom.txt",
                            "url": "http://jse.amstat.org/datasets/babyboom.dat.txt",
                            "filename": "babyboom",
                            "name": "Time of Birth, Sex, and Birth Weight of 44 Babies",
                            "separator": "TAB",
                            "columns": ["Time of birth recorded on the 24-hour clock", "Sex of the child (1 = girl, 2 = boy)", "Birth weight in grams", "Number of minutes after midnight of each birth"]
                        },
                        ...
                    ]
                },
                ...
            ]
        },
        ...
    ]
}
```

Note, the name of the collection specified here should be identical to the name of the json file.

To control the parsing process of the text file, several option are available. Below is the full list of parameters that can be specified to document the format of the data set:

```javascript
{
	"description": "description of the data set",
	"description_url": "URL to fetch the addition description",
	"url": "URL for the actual data",
	"filename": "name of the data file without the file extension",
	"name": "user-friendly name of the data set",
	"separator": "TAB",
	"comment_character": "#",
	"create_index_column": false,
	"skip_empty_parts": true,
	"simplify_whitespaces": false,
	"remove_quotes": true,
	"use_first_row_for_vectorname": true,
	"columns": [],
	"number_format": 1,
	"DateTime_format": "yyyy-MM-dd"
}
```
The only required parameters are *'description'*, *'filename'*, *'name'* and *'url'*. For other parameters default values apply as in the list below:

```javascript
"description_url": "",
"separator": "auto",
"comment_character": "#",
"create_index_column": false,
"skip_empty_parts": true,
"simplify_whitespaces": false,
"remove_quotes": true,
"use_first_row_for_vectorname": false,
"columns": [],
"number_format": 1,
"DateTime_format": ""
```

So, ommiting these parameters in the specification of the data set completely  still results in a complete description of the data set. The meaning of this parameters and their possible values are described in LabPlot's documentation.
