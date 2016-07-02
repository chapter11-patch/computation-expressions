= 第一歩: Maybeモナド

//lead{
"I call it my billion-dollar mistake. It was the invention of the null reference in 1965." Charles Antony Richard Hoare
//}

== null の問題点を探る

いくつかのプログラミング言語では、値が存在しないことを ``null`` キーワードで表現します。
そういった言語で意図せず ``null`` を束縛した変数にアクセスした場合、例外やエラーが発生します。
例外発生を回避するためには、事前に ``null`` チェックを行い、束縛されている値が ``null`` であることを確認する必要があります。
そして、この方法にはにはいくつかの問題点があります。

1. 利用者側はその関数を適用することによって ``null`` が束縛、代入される可能性を（すぐには）判別できない
2. ``null`` だった場合の処理を忘れやすい
3. ``null`` チェックによりネストが深くなりがち

ネストはともかく、ほかの二項目に関しては切実な問題なので、これらの問題の解決策としてよく知られている方法を見ていきましょう。
null 対する解決方法はいくつか存在します。

1. Null Object パターン
2. Safe Navigation Operator(groovy)、Null-propagating operator(C# 6.0)
3. 型による表現

Null Object パターンは、null の代わりに 値を持たない、処理が空のオブジェクトを使用するパターンです。
ただし、Null Object の実装次第では挙動が変わってしまうこともあるため、完全な代替とは言い切れません。
また、機械的な強制力はないため、初期化時の代入を忘れる可能性があります。

Safe Navigation Operator や Null-propagating operator は、左辺が null だった場合は null を返し、それ以外の場合は メソッドを呼び出す演算子です。
既存のコードをほとんど書き換えることなく利用できるので、便利なことは確かです。
しかし、 Null Object と同様に強制力はないため、うっかりミスは防げません。
また、この演算子が返す null がメソッドの戻り値として返される可能性は、0ではありません。

型による表現では、判別共用体(もしくはそれと同等の機能)によって "値を持たない状態" と "何かデータを持つ状態" を表します。
これにより、 値が存在しない可能性を型によって伝搬させることができ、この型を返す関数を利用するユーザーに対して何かしらの処理を行うことを強制できます。
なお、.NET Framework の Nullable も null を表現する型ですが、 Nullable は構造体のみを対象としています。
対して、判別共用体による表現では任意の型を対象にできます。

== 値が存在しない可能性を型で表す

それでは、値が存在するかもしれないことを型で表してみましょう。

//emlist{
type Maybe<'T> = Just of 'T | Nothing
//}

@<code>{Maybe} 型は、任意の値 を持つ @<code>{Just} と、値を持たない @<code>{Nothing} という2つのケース識別子から成ります。

//emlist{
let db = [ ("x", 1); ("y", 2); ("z", 3) ]
//}

ここで、 @<code>{db} から指定したキーに対応する値を取得する操作について考えてみましょう。
指定したキーが @<code>{db} に登録されていないときは、返す値がありません。
そこで、 @<code>{Maybe} 型の出番です。

//emlist{
let rec tryFind key db =
  match db with
  | [] -> Nothing
  | (k, v) :: xs when k = key -> Just v
  | _ :: xs -> tryFind key xs
//}

指定したキーに一致する値が見つからない際に @<code>{Nothing} を返すことで、値が存在しないことを伝えられるようになりました。

//emlist{
tryFind "y" db // equal (Just 2)
tryFind "w" db // equal Nothing
//}

@<code>{db} から二つの値を取得してその合計を返す関数を定義してみましょう。

//emlist{
let sum a b db =
  match tryFind a db with
  | Just x ->
    match tryFind b db with
    | Just y -> Just (x + y)
    | Nothing -> Nothing
  | Nothing -> Nothing

sum "x" "z" db // equal (Just 4)
sum "x" "w" db // equal Nothing
sum "v" "y" db // equal Nothing
//}

パターンマッチを使うことで、安全に値を取得して計算できるようになりました。

しかし、不満な点もあります。
パターンマッチがネストしてしまう点と、 @<code>{Nothing} にマッチした際 @<code>{Nothing} を返す、という定型的なコードが増えてしまう点です。

== 理想1: 定型的なコードを減らす

定型的なコードは、関数として解決できそうです。
ここで共通する操作は、 @<code>{Just} なら何かしらの計算を適用して @<code>{Maybe} 型を返し、 @<code>{Nothing} ならば @<code>{Nothing} を返す部分です。
今回は、演算子として抽出します。

//emlist{
// Maybe<'T> -> ('T -> Maybe<'U>) -> Maybe<'U>)
let (>>=) x f =
  match x with
  | Just v -> f v
  | Nothing -> Nothing
//}

@<code>{>>=} 演算子を用いて、 @<code>{sum} 関数を書き換えてみましょう。

//emlist{
let sum a b db =
  tryFind a db >>= fun x ->
  tryFind b db >>= fun y ->
  Just (x + y)
//}

不満点が解消されました。

== 理想2: 手続き的な書式で記述する

本章の序盤で "ネストはともかく" などと書いてしまいましたが、ネストを少なくしたほうが可読性の向上に繋がるのは確かです。
さらに欲を言えば、 @<code>{let} と同等のキーワードを利用して束縛を表現できれば、もっとわかりやすくなることでしょう。
F# では、コンピュテーション式でこれを実現します。

//emlist{
// コンピュテーション式用のビルダークラス
type MaybeBuilder () =
  member this.Return(x) = Just x
  member this.ReturnFrom(x: Maybe<_>) = x
  member this.Bind(x, f) = x >>= f

// ビルダーインスタンス
let maybe = MaybeBuilder()
//}

@<code>{>>=} に対応する @<code>{Bind} メソッドによって @<code>{let!} 、値を @<code>{Just} で包んで返す @<code>{Return} メソッドによって @<code>{return} 、 @<code>{Maybe} 型の値をそのまま返す @<code>{ReturnFrom} メソッドによって @<code>{return!} キーワードがコンピュテーション式内で利用可能になります。

準備が整ったので、 @<code>{sum} 関数を書き換えてみましょう。

//emlist{
let sum a b db =
  maybe {
    let! x = tryFind a db
    let! y = tryFind b db
    return x + y
  }
//}

コンピュテーション式の導入によって、ネストを平坦化することができました。

== 標準にあります

ここまで、 @<code>{Maybe} 型を独自に定義して Maybe モナドについて説明してきました。
しかし、この型を毎回定義するのは面倒くさいですね。

幸いなことに、 F# には @<code>{Maybe} 型と全く同じ動作をする @<code>{Option} 型が標準搭載されています。
@<code>{>>=} 演算子についても、 @<code>{bind} という関数名で定義されています。

というわけで、独自定義していた関数を組み込み関数で置き換えましょう。

//emlist{
let tryFind key db =
  List.tryFind (fst >> ((=) key)) db
  |> Option.map snd

let (>>=) x f = Option.bind f x

type OptionBuilder() =
  member this.Return(x) = Some x
  member this.ReturnFrom(x) = x
  member this.Bind(x, f) = x >>= f

let option = OptionBuilder()

let sum a b db =
  option {
    let! x = tryFind a db
    let! y = tryFind b db
    return x + y
  }

sum "x" "z" db // equal (Some 4)
sum "x" "w" db // equal None
sum "v" "y" db // equal None
//}

## 例題: ピエールの綱渡り

例題として、"すごい Haskell 楽しく学ぼう"に登場する例題"ピエールの綱渡り"を実装していきましょう。
以下に仕様を示します。

 1. ピエールが持つバランス棒の左右には鳥がとまることがある
 2. バランス棒の左右にとまっている鳥の差が 3 以下ならば、ピエールはバランスがとれる
 3. 差が 4 以上ならば、ピエールは安全ネットに落っこちる

まず、バランス棒にとまっている鳥の数を型で表しましょう。

//emlist{
type Birds = int
type Pole = Birds * Birds
//}

2-タプルの 第一要素が左側にとまっている鳥の数、第二要素が右側にとまっている鳥の数を表します。

次に、鳥の数をとって、バランス棒の左右に鳥をとまらせる関数を作ります。

//emlist{
// Birds -> Pole -> Pole
let landLeft n (left, right) =
  if abs ((left + n) - right) < 4 then Some (left + n, right)
  else None

// Birds -> Pole -> Pole
let landRight n (left, right) =
  if abs (left - (right + n)) < 4 then Some (left, right + n)
  else None
//}

鳥が飛び去る操作は、鳥の数に負数を渡すことで表現します。

それでは、ピエールに綱渡りさせてみましょう。

//emlist{
Some (0, 0) >>= landRight 2 >>= landLeft 2 >>= landRight 2 // equal (Some (2, 4))
//}

もう一回渡ってもらいましょう。

//emlist{
Some (0, 0) >>= landLeft 1 >>= landRight 4 >>= landLeft (-1) // equal None
//}

鳥が右側に集中してしまったことで、ピエールが落ちてしまいました。
落ちた状態で右側の鳥を減らしてみたらどうなるでしょう?

//emlist{
Some (0, 0) >>= landLeft 1 >>= landRight 4 >>= landLeft (-1) >>= landRight (-2) // equal None
//}

ピエールが落下してしまっていることが、後続の操作にも伝播していることがわかります。

最後に、コンピュテーション式を使って綱渡りをシミュレーションしてみましょう。

//emlist{
// routine equal (Some (3, 2))
let routine =
  option {
    let! start = Some (0, 0)
    let! first = landLeft 2 start
    let! second = landRight 2 first
    return! landLeft 1 second
  }
//}

