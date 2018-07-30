exports.index = (req, res, next) => {
  res.render('interview', {
    title: 'Interview'
  });
};
