exports.index = (req, res) => {
  res.render('home', {
    title: 'Home'
  });
};

exports.dashboard = (req, res) => {
  res.render('dashboard', {
    title: 'Dashboard'
  });
}
