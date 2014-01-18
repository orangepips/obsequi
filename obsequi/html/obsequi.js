// Domineering

// Simple 2 player game. One player is vertical, one horizontal, the first
// player who can't move loses.

// Might be interesting to implement an opponent...

// Canvas info.
var ctx; // Drawing context.
var block = 40; // Size of a "block".

// Game state.
var board;
var piece;

// Object for representing a domineering piece.
function Piece(row, col, vertical) {
  this.row = row;
  this.col = col;
  this.vertical = vertical;

  this.clone = function () {
    return new Piece(this.row, this.col, this.vertical);
  }
  this.blocks = function () {
    return [{
      row: this.row,
      col: this.col
    }, {
      row: this.row + (this.vertical ? 1 : 0),
      col: this.col + (this.vertical ? 0 : 1)
    }];
  }
  this.paint = function (outline) {
    var w = block * (this.vertical ? 1 : 2);
    var h = block * (this.vertical ? 2 : 1);
    if (outline) {
      ctx.strokeStyle = "red";
      ctx.strokeRect(this.col * block + 2, this.row * block + 2,
          w - 4, h - 4);
    } else {
      ctx.fillStyle = "blue";
      ctx.fillRect(this.col * block + 1, this.row * block + 1,
          w - 2, h - 2);
    }
  }
}

// Object for representing a domineering piece.
function Board(height, width) {
  this.height = height;
  this.width = width;
  this.board = [];
  for (var r = 0; r < height; r++) this.board.push([]);
  this.moves = [];

  this.paint = function () {
    ctx.fillStyle = "white";
    ctx.fillRect(0, 0, this.width * block, this.height * block);
    ctx.strokeStyle = "black";
    ctx.strokeRect(0, 0, this.width * block, this.height * block);

    // Paint the current board position.
    for (var r = 0; r < this.height; r++) {
      for (var c = 0; c < this.width; c++) {
        ctx.strokeStyle = "black";
        ctx.strokeRect(c * block, r * block, block, block);
      }
    }

    // Paint all the pieces.
    for (var i = 0; i < this.moves.length; i++) this.moves[i].paint();
  }
  this.in_bounds = function (piece) {
    var blocks = piece.blocks();
    for (var i = 0; i < blocks.length; i++) {
      var b = blocks[i];
      if (b.row < 0 || b.row >= this.height) return false;
      if (b.col < 0 || b.col >= this.width) return false;
    }
    return true;
  }
  this.valid = function (piece) {
    var blocks = piece.blocks();
    for (var i = 0; i < blocks.length; i++) {
      if (this.board[blocks[i].row][blocks[i].col]) return false;
    }
    return true;
  }
  this.make_move = function (piece) {
    this.moves.push(piece);

    var blocks = piece.blocks();
    for (var i = 0; i < blocks.length; i++) {
      this.board[blocks[i].row][blocks[i].col] = 1;
    }
  }
  this.undo_move = function() {
    var p = this.moves.pop();

    var blocks = p.blocks();
    for (var i = 0; i < blocks.length; i++) {
      this.board[blocks[i].row][blocks[i].col] = 0;
    }

    return p;
  }
  this.str = function() {
    var f = "";
    for (var r = 0; r < this.height; r++) {
      for (var c = 0; c < this.width; c++) {
        if (this.board[r][c]) {
           f += "" + (r) + "," + (c) + ";";
        }
      }
    }
    return f ? "b" + f : "";
  }
  this.get_real = function() {
    var count = 0;
    for (var r = 0; r < this.height; r++) {
      for (var c = 0; c < this.width - 1; c++) {
        if (!this.board[r][c] && !this.board[r][c+1]) {
           count++;
           c++;
        }
      }
    }
    return count;
  }
  this.get_safe = function() {
    var safe = function(board, row, col) {
      if (row > 0 && !board[row - 1][col]) return false;
      if (row < board.length - 1 && !board[row + 1][col]) return false;
      return !board[row][col];
    }
    var count = 0;
    for (var r = 0; r < this.height; r++) {
      for (var c = 0; c < this.width - 1; c++) {
        if (safe(this.board, r, c) && safe(this.board, r, c+1)) {
          count++;
          c++;
        }
      }
    }
    return count;
  }
  this.rotate = function() {
    var board = new Board(this.width, this.height);
    for (var i = 0; i < this.moves.length; i++) {
      var p = this.moves[i];
      board.make_move(new Piece(p.col, p.row, !p.vertical));
    }
    return board;
  }
  this.get_blocks = function() {
    var abc = [];
    for (var r = 0; r < this.height; r++) {
      for (var c = 0; c < this.width; c++) {
        if (this.board[r][c]) {
          abc.push({row:r,col:c});
        }
      }
    }
    return abc;
  }
}

function init() {
  var rows = $("#rows").val();
  var cols = $("#columns").val();

  var canvas = $("#canvas")[0];
  ctx = canvas.getContext("2d");
  ctx.fillStyle = "white";
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  block = Math.min(canvas.height/rows, canvas.width/cols);

  piece = new Piece(0, 0, true);
  board = new Board(rows, cols);

  update_score();

  paint();
}

function newgame() {
  init();
}

function solve() {
  var data = {board:{rows:board.height,
                     cols:board.width,
                     blocks:board.get_blocks(),
                     whos_turn:(piece.vertical ? "VERTICAL" : "HORIZONTAL")}};

  // Assign handlers immediately after making the request,
  // and remember the jqxhr object for this request
  var jqxhr = $.post( "xhr", JSON.stringify(data), function(d, t, r) {
    update_metrics(d);
    //alert(JSON.stringify(d));
  })
    .done(function() {
      //alert( "second success" );
    })
/*
    .fail(function() {
      alert( "error" );
    })
    .always(function() {
      alert( "finished" );
    });
 */
  // Perform other work here ...
}

function reverse() {
  var p = board.undo_move();
  piece = new Piece(0, 0, p.vertical);
  update_score();
  paint();
}

function canvasclick(e) {
  var x = e.hasOwnProperty('offsetX') ? e.offsetX : e.layerX;
  var y = e.hasOwnProperty('offsetY') ? e.offsetY : e.layerY;
  var p = piece.clone();
  p.col = Math.floor(x/block);
  p.row = Math.floor(y/block);
  
  if (board.valid(p)) {
    board.make_move(p);
    piece = new Piece(0, 0, !piece.vertical);
    update_score();
    paint();
  }
}

function set_item(id, text) {
  $(id).html(text);
  $(id).css("text-align", "right");
}

function update_metrics(data) {
  set_item("#vlower", data["v_metrics"]["moves_lower_bound"]);
  set_item("#hlower", data["h_metrics"]["moves_lower_bound"]);
  set_item("#vupper1", data["h_metrics"]["opp_moves_upper_bound"]);
  set_item("#hupper1", data["v_metrics"]["opp_moves_upper_bound"]);
  set_item("#vupper2", data["h_metrics"]["opp_moves_upper_bound2"]);
  set_item("#hupper2", data["v_metrics"]["opp_moves_upper_bound2"]);
}

function update_score() {
  var b = board.rotate();
  $("#vsafe").html(b.get_safe());
  $("#vsafe").css("text-align", "right");
  $("#vreal").html(b.get_real());
  $("#vreal").css("text-align", "right");
  $("#hsafe").html(board.get_safe());
  $("#hsafe").css("text-align", "right");
  $("#hreal").html(board.get_real());
  $("#hreal").css("text-align", "right");
  
  var x = "solve rows " + board.height + " cols " + board.width + " " +
      board.str() + (piece.vertical ? " V" : " H");
  $("#solve").html(x);
  solve();
}

function paint() {
  board.paint();
  piece.paint(true);
}

/*
// Lets add the keyboard controls now
$(document).keydown(function (e) {
  var key = e.which;

  var p = piece.clone();
  if (key == 37 / left / ) p.col--;
  else if (key == 39 / right / ) p.col++;
  else if (key == 38 / up / ) p.row--;
  else if (key == 40 / down / ) p.row++;

  if (board.in_bounds(p)) {
    piece = p;
  }

  if (key == 13 / return / ) {
    if (board.valid(piece)) {
      board.make_move(piece);
      solve();
      update_score();
      piece = new Piece(0, 0, !piece.vertical);
    }
  }

  paint();
})
*/

// Start once the html is all loaded.
$(document).ready(init);
