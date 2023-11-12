const express = require('express')
const cors = require('cors')
const app = express()
var bodyParser = require('body-parser');
var formidable = require('formidable');
var path = require('path');
var fs = require('fs-extra');
const fileUpload = require('express-fileupload');
var http = require('http');

app.use(cors())
app.use(express.json())

app.use(express.static('The Website'));

// START Does on start
//const nodeCmd = require('node-cmd');
//nodeCmd.run('a_db', (err, data, stderr) => console.log(data));
// END Does on start

app.get('/get_tables/', function (req, res)
{
  const nodeCmd = require('node-cmd');
  nodeCmd.run('a_db _get_tables', (err, data, stderr) => res.send(JSON.stringify(data)));
})

app.get('/cmd/', function (req, res)
{
  //res.send(JSON.stringify('a_db ' + req.query.statement));
  const nodeCmd = require('node-cmd');
  nodeCmd.run('a_db '+req.query.statement, (err, data, stderr) => res.send(JSON.stringify(data)));
})

app.use(fileUpload());

var uploaded_file_name = "";

app.post('/upload_csv/', function(req, res) 
{
  let sampleFile;
  let uploadPath;

  if (!req.files || Object.keys(req.files).length === 0) 
  {
    return res.status(400).send('No files were uploaded.');
  }

  // The name of the input field (i.e. "sampleFile") is used to retrieve the uploaded file
  sampleFile = req.files.sampleFile;
  uploadPath = __dirname + '/CSV_Uploads/' + sampleFile.name;

  uploaded_file_name = 'CSV_Uploads/' + sampleFile.name;

  // Use the mv() method to place the file somewhere on your server
  sampleFile.mv(uploadPath, function(err) {
    if (err)
      return res.status(500).send(err);

    res.send('File uploaded! Click the back button.');
  });
});

app.get('/create_csv/', function (req, res)
{
  if (uploaded_file_name != "")
  {
    //res.send(JSON.stringify('a_db _create_csv ' + uploaded_file_name +' '+ req.query.table_name +' '+ req.query.num_rows));
    const nodeCmd = require('node-cmd');
    nodeCmd.run('a_db _create_csv ' + uploaded_file_name +' '+ req.query.table_name +' '+ req.query.num_rows, (err, data, stderr) => res.send(JSON.stringify(data)));
  }
  else
    res.send(JSON.stringify("A file was not uploaded."));
  
})

app.listen(3000)

//nodeCmd.run('stop', (err, data, stderr) => console.log(data));
//process.exit(code=0)

//app.get('/stop/', function (req, res)
//{
  //process.exit(code=0)
//})