const path = require('path');
const dotenv = require('dotenv');
const cors = require('cors');
const express = require('express');
const compression = require('compression');
const session = require('express-session');
const logger = require('morgan');
const chalk = require('chalk');
const errorHandler = require('errorhandler');
const lusca = require('lusca');
const MongoStore = require('connect-mongo')(session);
const mongoose = require('mongoose');
const expressStatusMonitor = require('express-status-monitor');
const sass = require('node-sass-middleware');
const hbs = require('express-hbs');
const fs = require('fs');

const app = express();

try {
  const stats = fs.statSync('.env');
  console.log('File exists');
  dotenv.load({ path: '.env' });
} catch (err) {
  if (err.code == 'ENOENT') {
    dotenv.load({ path: '.env.example' });
  } else {
    console.log(err.code);
  }
}

const apiController = require('./controllers/api');
const homeController = require('./controllers/home');
const interviewController = require('./controllers/interview');
const authController = require('./controllers/auth');
const middlewares = require('./middlewares');
const passport = require('./config/passport');

mongoose.connect(
  process.env.MONGODB_URI,
  {
    useNewUrlParser: true
  }
);

mongoose.connection.on('error', err => {
  console.log(err);
  console.log(
    '%s MongoDB connection error. Please make sure MongoDB is running.',
    chalk.red('✗')
  );
  process.exit();
});

app.set('port', process.env.PORT || 8080);

hbs.registerHelper('equal', function(lvalue, rvalue, options) {
  if (arguments.length < 3) {
    throw new Error('Handlebars Helper equal needs 2 parameters');
  }
  if (lvalue != rvalue) {
    return options.inverse(this);
  } else {
    return options.fn(this);
  }
});

app.engine(
  'hbs',
  hbs.express4({
    partialsDir: path.join(__dirname, '/views/partials')
  })
);

app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'hbs');
app.use(expressStatusMonitor());
// app.use(
//   cors({
//     origin: process.env.CORS_ALLOW_ORIGIN || '*',
//     methods: ['GET', 'PUT', 'POST', 'DELETE', 'OPTIONS'],
//     allowedHeaders: ['Content-Type', 'Authorization']
//   })
// );

app.use(compression());
app.use(
  sass({
    src: path.join(__dirname, 'public'),
    dest: path.join(__dirname, 'public')
  })
);

if (process.env.NODE_ENV === 'development') {
  app.use(logger('dev'));
} else {
  app.use(logger('combined'));
}

app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(
  session({
    resave: true,
    saveUninitialized: true,
    secret: process.env.SESSION_SECRET,
    cookie: { maxAge: 1209600000 },
    store: new MongoStore({
      url: process.env.MONGODB_URI,
      autoReconnect: true,
      mongooseConnection: mongoose.connection
    })
  })
);

app.use(
  lusca({
    csrf: false,
    xframe: 'SAMEORIGIN',
    hsts: {
      maxAge: 31536000,
      includeSubDomains: true,
      preload: true
    },
    xssProtection: true,
    nosniff: true,
    referrerPolicy: 'same-origin'
  })
);

app.disable('x-powered-by');
app.use(passport.initialize());
app.use(passport.session());
app.use('/', express.static(path.join(__dirname, 'public'), { maxAge: 31557600000 }));
app.get('/dynamic_key', apiController.getDynamicKey);
app.get('/recording_key', apiController.getRecordingKey);
app.get('/dashboard', middlewares.checkAuthenticated, homeController.dashboard);
app.get('/interview', interviewController.index);
app.get('/login', authController.getLoginPage);
app.get('/register', authController.getSignupPage);
app.all('/logout', authController.logout);
app.post('/login', authController.postLoginData);
app.post('/register', authController.postSignupData);
app.get('/', middlewares.checkAuthenticated, homeController.index);
app.post('/question/create', middlewares.checkAuthenticated, apiController.createQuestion);
app.post('/question/update', middlewares.checkAuthenticated, apiController.updateQuestion);
app.post('/question/delete', middlewares.checkAuthenticated, apiController.deleteQuestion);
app.post('/interview/create', middlewares.checkAuthenticated, apiController.createInterview);
if (process.env.NODE_ENV === 'development') {
  app.use(errorHandler());
}

app.listen(app.get('port'), () => {
  console.log(
    `${chalk.green('✓')} App is running at http://localhost:${app.get('port')} in ${app.get('env')} mode`    
  );
  console.log('  Press CTRL-C to stop\n');
});

module.exports = app;
