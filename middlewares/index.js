module.exports = exports = {
  checkAuthenticated: (req, res, next) => {
    if (req.user) {
      return next();
    }

    res.status(404).redirect('/login');
  }
};
