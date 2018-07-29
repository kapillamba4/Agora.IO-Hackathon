const User = require('../models/User');
const passport = require('../config/passport');

exports.getLoginPage = (req, res) => {
  if (req.user) {
    return res.redirect('/dashboard');
  }

  res.render('login', {
    title: 'Login'
  });
};

exports.postLoginData = (req, res, next) => {
  passport.authenticate('local', (err, user, info) => {
    if (err) {
      return next(err);
    }

    if (!user) {
      return res.redirect('/login');
    }

    req.logIn((user, err) => {
      if (err) {
        return next(err);
      }

      res.redirect(req.session.returnTo || '/');
    });
  })(req, res, next);
};

exports.logout = (req, res) => {
  req.logout();
  req.session.destroy(err => {
    if (err) console.log('Error : Failed to destroy the session during logout.', err);
    req.user = null;
    res.redirect('/');
  });
};

exports.getSignupPage = (req, res) => {
  if (req.user) {
    return res.redirect('/dashboard');
  }

  res.render('register', {
    title: 'Sign Up'
  });
};

exports.postSignupData = (req, res, next) => {
  User.findOne({ email: req.body.email }, (err, existingUser) => {
    if (err) {
      console.log(err);
      return next(err);
    }

    if (existingUser) {
      return res.redirect('/signup');
    }

    const user = new User({
      email: req.body.email,
      password: req.body.password
    });

    user.save(err => {
      if (err) {
        console.log(err);
        return next(err);
      }

      req.logIn(user, err => {
        if (err) {
          console.log(err);
          return next(err);
        }

        res.redirect('/dashboard');
      });
    });
  });
};
