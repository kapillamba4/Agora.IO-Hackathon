const mongoose = require('mongoose');

const questionSchema = new mongoose.Schema(
    {
        email: { type: String, unique: true },
        question: { type: String, unique: true },
        possibleAnswers: [String]
    },
    { timestamps: true }    
);

const Question = mongoose.model('Question', questionSchema);

module.exports = exports = Question;